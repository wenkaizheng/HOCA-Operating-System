#ifndef PROCQ
#define PROCQ
typedef struct proc_link{
    int index;
    struct proc_t *next;
}proc_link;

/* process table entry type */
typedef struct proc_t{
    proc_link p_link[SEMMAX];  /* pointer to the next entries */
    state_t  p_s;  /* processor state of the process */
    int qcount;  /* number of queues containing this entry */
    int *semvec[SEMMAX]; /* vector of active semaphores for this entry */
}proc_t;

#endif