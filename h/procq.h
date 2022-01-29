#ifndef PROCQ
#define PROCQ
typedef struct proc_link{
    int index;
    struct proc_t *next;
}proc_link;

typedef struct proc_t{
    proc_link p_link[SEMMAX];
    state_t  p_s;
    int qcount;
    int *semvec[SEMMAX];
}proc_t;

#endif