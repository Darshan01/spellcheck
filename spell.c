#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>

#include "spell.h"

//custom character comparison function
int charcmp(char dictC, char wordC, int capMode){
    
    //the letter in the dictionary is uppercase
    //the letter in the word must match exactly
    if(isupper(dictC)){
        return dictC - wordC;
    }

    //the letter in the dictionary is lower case
    if(islower(dictC)){

        //word has first letter capitalized, or all letters capitalized
        if(capMode == 0){

            //the letter in the word is uppercase
            if(isupper(wordC)){
                
                //if the lowercase version matches the dictionary, they match
                if(dictC - tolower(wordC) == 0) return 0;
                else return 1; //otherwise, say the dictionary letter is larger

            } else {
                //dictionary and word letter are lowercase, return the difference
                return (dictC - wordC);

            }

        } else {
            //word has mixed capitalization, it should match dictionary word exactly
            return (dictC - wordC);
        }

    }

    //for non alpha characters
    return dictC - wordC;
}

//searches the dictionary for the word
//left and right are binary search variables
//wordCounter and dictCounter hold the place of the character in the word or the dictionary entry
//cap mode determines how the word is capitalized (initial, all caps, or neither)
int searchDict(char* word, alSTR_t *dictionary, int left, int right, int wordCounter, int dictCounter,int capMode){
    
    int middle; //binary search variable
    int cmp; //holds value returned by charcmp()
    
    //while our left bound is less than or equal to our right bound
    //continue binary search
    while(left <= right) {

        middle = left + (right - left) / 2; //redefine middle
        
        //if either the dictionary entry or word have an apostrophe
        //go to the next letter for comparison
        if(dictionary->data[middle][dictCounter] == '\'' && word[wordCounter] == '\''){
            dictCounter++;
            wordCounter++;
        } else if(dictionary->data[middle][dictCounter] == '\''){
            dictCounter++;
        } else if(word[wordCounter] == '\''){
            wordCounter++;
        }

        //compare characters
        cmp = charcmp(dictionary->data[middle][dictCounter], word[wordCounter], capMode);

        //if the characters are the same
        if(cmp == 0){
            
            //if we are at the end of the dictionary entry
            if(dictCounter == strlen(dictionary->data[middle]) - 1){
                
                //if we are at the end of the word
                //strcmp is used to ensure that words with apostrophes have the apostrophes in the right place
                if(wordCounter == strlen(word) - 1 && strcmp(dictionary->data[middle], word) == 0){
                    return 0; //word is found
                }

                //if we are not at the end of the word
                //word is longer than dictionary entry, go to the right for binary search
                return searchDict(word, dictionary, middle + 1, right, 0, 0, capMode);


            } else {
                //we are not at the end of the dictionary entry

                //the word is shorter than the dictionary entry
                if(wordCounter == strlen(word) - 1){
                    //search the left of the dictionary
                    return searchDict(word, dictionary, left, middle - 1, 0, 0, capMode);
                
                } else {
                    //we are not at the end of either
                    //call this function recursively to find the next character
                    return searchDict(word, dictionary, left, right, ++wordCounter, ++dictCounter, capMode);
                }
            }
        }

        //dictionary character is less than the word character
         else if(cmp < 0){
            //go to the right of the dictionary and reset the counters
            left = middle + 1;
            wordCounter = 0;
            dictCounter = 0;
        }

        //dictionary character is greater than the word character
         else if(cmp > 0){
            //go to the left of the dictionary and reset the counters
            right = middle - 1;
            wordCounter = 0;
            dictCounter = 0;
        }
    }

    //if the first or all letters in the word are capitalized,
    //or the dictionary is not sorted, binary search may fail
    //in this case, do a linear search
        
    //for each entry in the dictionary
    for(int i = 0; i < dictionary->length; i++){
        wordCounter = 0;
        dictCounter = 0;

        //while we are not at the end of either word
        while(wordCounter != strlen(word) || dictCounter != strlen(dictionary->data[i])){

            //if the characters are the same, go to the next character
            //otherwise, go to the next dictionary entry
            if(charcmp(dictionary->data[i][dictCounter], word[wordCounter], capMode) == 0){
                wordCounter++;
                dictCounter++;
            } else {
                break;
            }

            //if we have reached the end of the word and the dictionary entry, we have a match
            if(wordCounter == strlen(word) && dictCounter == strlen(dictionary->data[i])) return 0;
        }
    }
    
    //all searches failed
    return 1;
}

//checks if character is acceptable to start a word
int notAcceptable(char c){
    return isspace(c) || c == '(' || c == '[' || c == '{' || c == '\'' || c == '\"';
}

//check the spelling of char *word using alSTR_T *dictionary
int spellCheck(char *word, char *file, int l, int c, alSTR_t *dictionary){
    int wordLength = strlen(word);

    int capMode = 1; //indicates if the word is all caps, initial caps, or mixed
    int caps = 0; //counts number of non-lowercase letters
    int wordStart = 0; //keeps track of the start of each word in hyphenated string
    int wordEnd = 0; //keeps track of end of each word in hyphenated string
    char *tempWord; //stor temporary string

    int numErrors = 0; //spelling errrors

    //cut off the string at the last alpha character

    //find last alpha character
    for(int i = 0; i < wordLength; i++){
        if(isalpha(word[i])){
            wordEnd = i;
        }
    }

    //insert null terminator after last character
    word[wordEnd + 1] = '\0';
    wordLength = strlen(word); //redefine word length

    //loop through each character in the word
    for(int i = 0; i < wordLength; i++){
        //if the letter is not lowercase, count it as a capital
        if(!islower(word[i])) caps++;

        //if there is a hyphen
        if(word[i] == '-' && (i != 0 && !notAcceptable(word[i-1]))){

            caps--; //discount the last hyphen as a capital

            //create a temporary word to hold the word from the start to before the hyphen
            //copy the word up to that point, add a null terminator
            tempWord = malloc((i - wordStart + 1) * sizeof(char));
            strncpy(tempWord, word + wordStart, i - wordStart + 1);
            tempWord[i - wordStart] = '\0';

            //if the first letter is capitalized, or all caps, set the capitalization mode
            if((isupper(tempWord[0]) && caps == 1) || caps == strlen(tempWord)){
                capMode = 0;
            } 

            caps = 0; //reset number of caps

            //search for the word in the dictionary
            numErrors += searchDict(tempWord, dictionary, 0, dictionary->length - 1, 0, 0, capMode);
            
            //the next word starts after the hyphen
            wordStart = i + 1;
            free(tempWord);            
        }
    }

    //for words with out hyphens, or the last word in a hyphenated string
    if((isupper((word+wordStart)[0]) && caps == 1) || caps == strlen(word+wordStart)){
        capMode = 0;
    } 

    //if the last character is a letter, the string is a valid word
    if(isalpha(word[wordEnd])) numErrors += searchDict(word + wordStart, dictionary, 0, dictionary->length - 1, 0, 0, capMode);
    
    //if a spelling error was found, print the error with the file, line #, and column #
    if(numErrors){
        fprintf(stderr, "%s (%i, %i): %s\n", file, l, c, word);
    }
    return numErrors;
    
}