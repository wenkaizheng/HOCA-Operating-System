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
    int cpu_time;
    long start_time;
    state_t* prog_old;
    state_t* prog_new;
    state_t* memory_management_old;
    state_t* memory_management_new;
    state_t* system_call_old;
    state_t* system_call_new;
    struct proc_t* parent;
    struct proc_t* child;
    struct proc_t* brother;
}proc_t;

#endif