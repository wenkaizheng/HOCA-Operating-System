// This code is my own work, it was written without
// consulting a tutor or code written by other students-Wenkai Zheng
#include "../h/const.h"
#include "../h/types.h"
#include "../h/procq.h"

proc_t proc_table[MAXPROC];
proc_t * free_head;
char pncbuf[512];

void panic(sp)
    register char *sp;
{
        register char *ep = pncbuf;
        while ((*ep++ = *sp++) != '\0');
        asm("	trap	#0");
}

proc_t* headQueue(proc_link tp){
    if (tp.next == (proc_t*) ENULL){
        return (proc_t*) ENULL;
    }
    return tp.next->p_link[tp.index].next;
}
proc_t* allocProc(){
    if (free_head == (proc_t *)ENULL) {
        return (proc_t *)ENULL;
    }
    proc_t* rv  = free_head;
    free_head = free_head->p_link[0].next;
    rv->p_link[0].index = ENULL;
    rv->p_link[0].next = (proc_t *)ENULL;
    return rv;
}
int initProc(){
    int i;
    int j;
    for (i = 0; i< MAXPROC; i++){
        for (j = 0; j< SEMMAX; j++){
            proc_table[i].p_link[j].index =  ENULL;
            proc_table[i].p_link[j].next = (proc_t *) ENULL;
            proc_table[i].semvec[j] = (int *)ENULL;
        }
        proc_table[i].qcount = 0;
    }
    for(i = 0;i < MAXPROC -1; i++){
        proc_table[i].p_link[0].index = 0;
        proc_table[i].p_link[0].next = &(proc_table[i+1]);
    }
    proc_table[MAXPROC - 1].p_link[0].index = ENULL;
    proc_table[MAXPROC - 1].p_link[0].next = (proc_t *) ENULL;
    free_head = &(proc_table[0]);
    return 0;
}
int insertProc(proc_link* tp, proc_t* p){
    proc_t* tail = tp->next;
    int tail_index = tp->index;
    int i;
    for (i = 0; i< SEMMAX; i++){
        if (p->p_link[i].index == ENULL){
            break;
        }
    }
    if ( i == SEMMAX){
        panic("This process is already in 20 sem queue\n");
    }
    // after i is found, we have 2 cases
    // when it is first insert or not
    if (tail == (proc_t*) ENULL){
        // first time
        p->p_link[i].index = i;
        p->p_link[i].next = p;
    }else{
        // not first time
        // p next point to head
        p->p_link[i].index = tail->p_link[tail_index].index;
        p->p_link[i].next = tail->p_link[tail_index].next;
        // prev tail point to p
        tail->p_link[tail_index].index = i;
        tail->p_link[tail_index].next = p;
    }
    // all need to do whether it is first time or not
    p->qcount ++;
    tp->index = i;
    tp->next = p;
    return 0;
}
proc_t* removeProc(proc_link* tp){
    if (tp -> next == (proc_t*) ENULL){
        return (proc_t*) ENULL;
    }
    proc_t* rv;
    // if it is only one in the queue
    if (tp->next == headQueue(*tp)){
        rv = tp->next;
        rv -> p_link[tp->index].index = ENULL;
        rv -> p_link[tp->index].next = (proc_t *)ENULL;
        tp->index = ENULL;
        tp->next = (proc_t *)ENULL;
    }else{
        // morn than 1 node in the queue
        rv = headQueue(*tp);
        int head_index = tp->next->p_link[tp->index].index;
        // find the head -> next and use tp->next to point to it
        int new_head_index = rv->p_link[head_index].index;
        proc_t* new_head = rv->p_link[head_index].next;

        tp->next->p_link[tp->index].index = new_head_index;
        tp->next->p_link[tp->index].next = new_head;
        rv->p_link[head_index].index = ENULL;
        rv->p_link[head_index].next = (proc_t *)ENULL;
    }
    // all need to do whether it is first time or not
    rv->qcount --;
    return rv;

}

int freeProc(proc_t* p){
    int i;
    for (i=0;i<SEMMAX;i++){
        p->p_link[i].next = (proc_t *)ENULL;
        p->p_link[i].index = ENULL;
        p->semvec[i] = (int *)ENULL;
    }
    p->qcount = 0;
    if (free_head == (proc_t *)ENULL){
        free_head = p;
    }else{
        // insert to the tail
        proc_t* walker = free_head;
        while(walker->p_link[0].next != (proc_t *)ENULL){
            walker = walker->p_link[0].next;
        }
        walker->p_link[0].next = p;
    }
    return 0;
}

proc_t* outProc(proc_link *tp, proc_t *p){
    if (tp->next == (proc_t *)ENULL){
        return (proc_t *)ENULL;
    }
    // if the head == p and there is only one node in the queue
    proc_t* rv;
    if (tp->next == headQueue(*tp) && tp->next == p){
        rv = tp->next;
        rv->p_link[tp->index].index = ENULL;
        rv->p_link[tp->index].next = (proc_t *)ENULL;
        tp->index = ENULL;
        tp->next = (proc_t *)ENULL;
        rv->qcount --;
        return rv;
    }
    // if the head != p and there is only one node in the queue
    if (tp->next == headQueue(*tp) && tp->next != p){
        return (proc_t *)ENULL;
    }
    int prev_index = tp->index;
    proc_t* pre_proc_t = tp->next;
    int cur_index = tp->next->p_link[tp->index].index;
    proc_t* cur_proc_t = tp->next->p_link[tp->index].next;
    // go from head to tail (exclude tail)
    while (cur_proc_t != tp->next){
        // in the middle we make prev point to cur->next
        if (cur_proc_t == p){
           pre_proc_t->p_link[prev_index].index = cur_proc_t->p_link[cur_index].index;
           pre_proc_t->p_link[prev_index].next = cur_proc_t->p_link[cur_index].next;
           rv = cur_proc_t;
           rv->p_link[cur_index].index = ENULL;
           rv-> p_link[cur_index].next =  (proc_t *)ENULL;
           rv -> qcount --;
           return rv;
        }
        // update prev and cur
        prev_index = cur_index;
        pre_proc_t = cur_proc_t;
        int copy_index = cur_index;
        cur_index = cur_proc_t->p_link[cur_index].index;
        cur_proc_t = cur_proc_t->p_link[copy_index].next;

    }
    // tail was found as p
    if ( cur_proc_t == tp->next && cur_proc_t == p){
        pre_proc_t->p_link[prev_index].index = cur_proc_t->p_link[cur_index].index;
        pre_proc_t->p_link[prev_index].next = cur_proc_t->p_link[cur_index].next;
        rv = cur_proc_t;
        rv->p_link[cur_index].index = ENULL;
        rv-> p_link[cur_index].next =  (proc_t *)ENULL;
        rv -> qcount --;
        tp->index = prev_index;
        tp->next = pre_proc_t;
        return rv;
    }
    // not found in any position
    if (cur_proc_t == tp->next && cur_proc_t!= p){
        return (proc_t *)ENULL;
    }
    return (proc_t *)ENULL;
}