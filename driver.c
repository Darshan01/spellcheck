#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>

#include "driver.h" 
#include "spell.h"
#include "arraylist.h"

#define BUFLENGTH 16

int main(int argc, char **argv){
    //at least one dictionary and one file/directory must be provided
    if(argc < 3){
        fprintf(stderr, "Please provide a dictionary and files to spell check in the following format:\n./spchk path/to/dictionary fileOrDirectory [additional files or directories]\n");
        return EXIT_FAILURE;
    }

    //variable to determine if any spelling errors were found
    int spellingErrors = 0;

    //must provide readable dictionary file with usable file extensions
    if(isFile(argv[1]) == -1){
        fprintf(stderr, "Please provide a dictionary text file that can be opened\n");
        return EXIT_FAILURE;

    } else if(isFile(argv[1]) != 0){
        fprintf(stderr, "Please provide a dictionary file with an extension such as .txt\n");
        return EXIT_FAILURE;

    }
    
    //open dictionary file and initialize dictionary arraylist
    //(see alSTR.c/h)
    int dictFD = open(argv[1], O_RDONLY);
    alSTR_t dict;
    alSTR_init(&dict, BUFLENGTH);
    
    //store the dictionary file into the arraylist
    readDict(dictFD, &dict);

    //loop through all the given arguments
    for(int i = 2; i < argc; i++){

        if(isFile(argv[i]) == -1){
            //stat could not open the file/directory

        } else if(isFile(argv[i]) == 0){
            //open regular file, read through it and check spelling of each word
            int fd = open(argv[i], O_RDONLY);
            spellingErrors += readFile(fd, argv[i], &dict);
            close(fd);

        } else if(isFile(argv[i])  == 2){
            //file is a directory, go through all entries
            int travDirRes = travDir(argv[i], &dict);
            if(travDirRes != -1) spellingErrors += travDirRes;
            
        }
    }

    //free the dictionary
    alSTR_destroy(&dict);

    //if any spelling errors were found, return EXIT_FAILURE
    if(spellingErrors){
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

//uses stat function to determine file type
int isFile(char *path){
    struct stat status;

    //stat() failed, file could not be opened
    if(stat(path, &status) == -1){
        fprintf(stderr, "file/directory could not be opened: %s\n", path);
        return -1;
    }

    //S_ISREG macro tells us if the file is a regular file
    if(S_ISREG(status.st_mode)){
        return 0;

    //S_ISDIR tells us if the file is a directory
    } else if (S_ISDIR(status.st_mode)) {
        return 2;
    }

    return 3; //any othe file type
}

//traverses directories and all sub-directories
int travDir(char *path, alSTR_t *dict){

    int numErrors = 0; //indicate if there are any spelling errors
    DIR *current = opendir(path); //open directory given by path

    //opendir() failed
    if(current == NULL){
        fprintf(stderr, "directory could not be opened: %s\n", path);
        return -1;

    }
    
    struct dirent *read;
    al_t fullPath; //arraylist to hold the path of each file (see arraylist.c/h)
    
    //while there are still entries in the directory
    while((read = readdir(current))){

        //ignore files starting with a .
        if(read->d_name[0] == '.'){
            continue;

        }

        //initialize arraylist and push the path passed into the function
        //push a slash, then the name of the entry in the directory given by path
        al_init(&fullPath, 2*strlen(path));
        al_pushStr(&fullPath, path);
        al_push(&fullPath, '/');
        al_pushStr(&fullPath, read->d_name);
        

        //if the entry is a directory, call this function recursively with the path to that directory
        if(isFile(fullPath.data) == 2){
            travDir(fullPath.data, dict);
            al_destroy(&fullPath);
            continue;

        }

        //if the file is a regular file
        if(isFile(fullPath.data) == 0){
            //find the extension of the file
            char* test = strchr(read->d_name, '.');

            //if the file has an extension and it is a text file
            if(test != NULL && strcmp(test, ".txt") == 0){

                //open the file and check the spelling of each word
                int fd = open(fullPath.data, O_RDONLY);
                numErrors += readFile(fd, fullPath.data, dict);

            }

        }

        al_destroy(&fullPath);
    }

    closedir(current);
    return numErrors;
}

//stores the dictionary
int readDict(int dictFD, alSTR_t *dict){
    //define buffer length and malloc space for it
    int buflength = BUFLENGTH;
    char *buffer = malloc(buflength * sizeof(char));

    int bytesRead; //variable to hold the number returned by read()
    int bytesLeft = 0; //bytes unread in the buffer
    int i = 0; //iterator variable

    //read through the file
    //write to where we left off in the buffer
    //only write enough to fill the buffer
    while((bytesRead = read(dictFD, buffer + i, buflength - i))){

        //read() failed
        if(bytesRead == -1){
            fprintf(stderr, "There was an error reading the dictionary file\n");
            exit(EXIT_FAILURE);
        }

        //add how many bytes read to the count of bytes that are left in buffer
        bytesLeft += bytesRead;
        
        //if read() returns less than we request, we are at end of file
        if(bytesRead < buflength - i){
            break;
        }

        //while there are bytes left in the buffer
        while(bytesLeft){

            //if the character is not a newline, keep going
            if(buffer[i] != '\n'){
                i++;
                bytesLeft--;

                //if we are at the end of our buffer, double the size and read again
                if(i == buflength){
                    buflength *= 2; 
                    char* temp = realloc(buffer, buflength);

                    if(temp == NULL){
                        fprintf(stderr, "No memory available for realloc\n");
                        exit(EXIT_FAILURE);
                    }

                    buffer = temp;
                    break;
                }

                continue;
            }

            //found a new line, replace it with null terminator
            buffer[i] = '\0';
            bytesLeft--;

            //push the word into our dictionary arraylist
            alSTR_push(dict, buffer);

            //if there are still bytes left in the buffer
            //move the point where we left off to the start of the buffer
            //read() will fill in the rest.
            if(bytesLeft){
                memmove(buffer, buffer + i + 1, bytesLeft);
            }

            //go back to the start of the buffer.
            i = 0;
        }    
    }

    //code to handle end of file
    
    while(bytesLeft){

        //if we find a new line
        if(buffer[i] == '\n'){

            //insert a null terminator and push to our dictionary arraylist
            buffer[i] = '\0';
            alSTR_push(dict, buffer);

            bytesLeft--;
            if(bytesLeft <= 0){
                break;
            }

            //shift the unread part of buffer to the start and reset our iterator
            memmove(buffer, buffer + i + 1, bytesLeft);
            i = 0;
        }
        
        i++;
        bytesLeft--;
    }

    //the last word will not end with a new line

    //if the last word took up the entire buffer
    //allocate one more byte for the null terminator
    if(i == buflength){
        buffer = realloc(buffer, buflength + 1);
        if(buffer == NULL){
            fprintf(stderr, "No memory available for realloc\n");
            exit(EXIT_FAILURE);
        }
    }
    
    //add null terminator for last word and push into dictionary
    buffer[i] = '\0';

    alSTR_push(dict, buffer);

    free(buffer);
    return 0;
}

//checks spelling of each word in the given file
int readFile(int fd, char* fileName, alSTR_t *dict){

    //define buffer length and malloc space for it
    int buflength = BUFLENGTH;
    char *buffer = malloc(buflength * sizeof(char));

    //arraylist to hold the word being read
    al_t word;
    al_init(&word, BUFLENGTH);

    int bytesRead = 1; //number returned by read()
    int bytesLeft = 0; //bytes left to read in the buffer
    int i = 0; //iterator

    int lineNum = 1; //line number of the word
    int columnNum = 1; //column number of the word

    int numErrors = 0; //tracks spelling errors

    int wordStart = 0; //store the index of the first letter in a word

    //while bytes are left in the file
    while((bytesRead = read(fd, buffer + i, buflength - i))){

        //read() failed
        if(bytesRead == -1){
            fprintf(stderr, "There was an error reading the file: %s\n", fileName);
            exit(EXIT_FAILURE);
        }

        //increase the number of bytes left by the number of bytes read
        bytesLeft += bytesRead;

        //while bytes are left in the buffer
        while(bytesLeft){

            //if the character is not a letter, keep iterating
            if(notAcceptable(buffer[i])){
                
                if(word.length != 0){
                    //this case is if our buffer ended in the middle of the word
                    //and we needed read() to read more bytes to finish the word
                    //the word arraylist still has some characters in it, so go to READWORD to finish reading
                    // int wordStart = i; //store the index of the first letter in a word
                    goto READWORD;
                }

                wordStart = i+1; //word starts on next character

                
                columnNum++;
                if(buffer[i] == '\n') {
                    //found a newline, increment the line number and reset the column number
                    lineNum++;
                    columnNum = 1;
                }

                i++;
                bytesLeft--;

                //if we reached the end of our buffer, call read() again
                if(i == buflength){
                    i = 0;
                    break;
                }
                
                
                continue;
            }

            //found a letter
            READWORD:
            int wordEnd = 0; //store the index of the last letter in a word
            int currentWordStart = i; //stores the start of the word if the buffer ended up cutting off a word

            //iterate until we encounter a whitespace
            while(bytesLeft && !isspace(buffer[i])){
                //if the character is a letter, update wordEnd
                if(!isspace(buffer[i])){
                    wordEnd = i+1;
                }

                i++;
                bytesLeft--;
            }

            //store the word from the first letter to the last letter
            //including any non alpha characters in between
            if(wordEnd != 0){
                for(int j = currentWordStart; j < wordEnd; j++){
                    al_push(&word, buffer[j]);
                }
            }
            
            //if we reached the end of the buffer, our word may have been cut off
            //double the size of the buffer with realloc and go back to read()
            if(bytesLeft == 0){
                if(i == buflength){

                    buflength *= 2; 
                    char* temp = realloc(buffer, buflength);

                    if(temp == NULL){
                        fprintf(stderr, "No memory available for realloc\n");
                        exit(EXIT_FAILURE);
                    }

                    buffer = temp;
                }
                continue;
            }
            
            //not at the end of our buffer, we found a complete word

            //check spelling of the word
            numErrors += spellCheck(word.data, fileName, lineNum, columnNum, dict);
            columnNum += (i - wordStart); //iterate the column number by the length of the word

            //destroy and re-initialize the arraylist storing our word
            al_destroy(&word);
            al_init(&word, buflength);
            
            //if we are not at the end of the buffer
            //shift the data we have not read to the start
            if(bytesLeft){
                memmove(buffer, buffer + i, bytesLeft);
            }

            i = 0;
        }
    }

    //if there was a word at the end of the file, check it spelling
    if(word.length){
        numErrors += spellCheck(word.data, fileName, lineNum, columnNum, dict);
    }
    
    //destroy the arraylist holding our word, free the buffer
    al_destroy(&word);
    free(buffer);
    return numErrors;
}