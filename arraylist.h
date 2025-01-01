//arraylist of characters
typedef struct {
    char* data;
    unsigned length;
    unsigned capacity;
} al_t;

void al_init(al_t *, unsigned);
void al_destroy(al_t *);

void al_push(al_t *, char);
void al_pushStr(al_t *, char*);

