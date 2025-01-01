#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arraylist.h"

//initialize empty arraylist
void al_init(al_t *L, unsigned size){

    if(size < 0){
        fprintf(stderr, "Cannot initialize arraylist with negative sizea: %i\n", size);
        exit(EXIT_FAILURE);
    }

    L->data = malloc(size * sizeof(char) + 1);

    if(L->data == NULL){
        fprintf(stderr, "Not enough memory available for malloc in al_init\n");
        exit(EXIT_FAILURE);
    }

    L->length = 0;
    //start off the arraylist with a null terminator
    //this ensures al_t.data will always will always be a null terminated string
    L->data[0] = '\0';
    L->capacity = size;
}

//free arraylist
void al_destroy(al_t *L){
    free(L->data);
}

//push a character into the arraylist
void al_push(al_t *L, char item){

    //if the length, plus null terminator, is equal to capacity
    //double the capacity with realloc
    if(L->length+1 == L->capacity){
        L->capacity *= 2;
        char* temp = (char*) realloc(L->data, L->capacity * sizeof(char));
        
        if(temp == NULL){
            fprintf(stderr, "Not enough memory available for realloc in al_push\n");
            exit(EXIT_FAILURE);

        }

        L->data = temp;
    }

    //insert the character to the end of the arraylist
    L->data[L->length] = item;
    //put the null terminator after that
    L->data[L->length+1] = '\0';
    L->length++;
}

//pushes a string character by character into the arraylist
void al_pushStr(al_t *L, char *item){

    //loop through characters in char *item
    for(int i = 0; i < strlen(item); i++){
        al_push(L, item[i]); //push each character with al_push
    }
}

