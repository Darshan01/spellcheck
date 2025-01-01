#ifndef alSTR
#define alSTR
#endif //used to avoid double inclusion of this file

//arraylist of strings
typedef struct {
    char** data;
    unsigned length;
    unsigned capacity;
} alSTR_t;

void alSTR_init(alSTR_t *, unsigned);
void alSTR_destroy(alSTR_t *);

void alSTR_push(alSTR_t *, char*);
