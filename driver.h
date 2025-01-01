#ifndef alSTR
#include "alSTR.h"
#endif

int isFile(char *);
int travDir(char *, alSTR_t *);
int readDict(int, alSTR_t *);
int readFile(int, char*, alSTR_t *);