// This code is my own work, it was written without
// consulting a tutor or code written by other students-Wenkai Zheng
#include "../h/const.h"
#include "../h/types.h"
#include "../h/procq.e"
#include "../h/asl.e"

semd_t semd_table[MAXPROC];
semd_t* cur_sem;
semd_t* free_head_s;

void setSem(int *semAdd, proc_t *p){
    int i;
    for(i = 0; i<SEMMAX; i++){
        if (p->semvec[i] == (int*)ENULL){
            p->semvec[i] = semAdd;
            break;
        }
    }
}
void setSem2(int *semAdd, proc_t *p){
    int i;
    for(i = 0; i<SEMMAX; i++){
        if (p->semvec[i] == semAdd){
            p->semvec[i] = (int*)ENULL;
            break;
        }
    }
}
void removeBlockHelper(semd_t* walker){
    // if it is empty, delete it and add it to free list
    // we have 4 case in here
    // if walker has both prev and next
    if (walker->s_prev != (semd_t *) ENULL && walker->s_next != (semd_t *) ENULL){
        walker->s_prev->s_next = walker->s_next;
        walker->s_next->s_prev = walker->s_prev;
    }// if walker has prev but not next
    else if (walker->s_prev != (semd_t *) ENULL && walker->s_next == (semd_t *) ENULL){
        walker->s_prev->s_next = (semd_t *) ENULL;
    }// if walker has next but not prev which means it is head(cur_sem need to point ot next)
    else if (walker->s_prev == (semd_t *) ENULL && walker->s_next != (semd_t *) ENULL){
        walker->s_next->s_prev = (semd_t *) ENULL;
        cur_sem = walker->s_next;
    }// if walker does not have next and prev
    else{
        cur_sem = (semd_t *) ENULL;
    }
    if (free_head_s == (semd_t *) ENULL){
        walker->s_prev = (semd_t *) ENULL;
        walker->s_next = (semd_t *) ENULL;
        walker->s_semAdd = (int*) ENULL;
        free_head_s = walker;
    }else{
        semd_t* walker2 = free_head_s;
        // insert to end of walker2
        while(walker2->s_next!= (semd_t*)ENULL){
            walker2 = walker2->s_next;
        }
        walker2->s_next = walker;
        walker->s_prev = walker2;
        walker->s_next = (semd_t *) ENULL;
        walker->s_semAdd = (int*) ENULL;
    }
}
int insertBlocked(int *semAdd, proc_t *p){
    if (cur_sem == (semd_t *)ENULL){
        cur_sem = free_head_s;
        free_head_s = free_head_s->s_next;
        cur_sem->s_next =  (semd_t *)ENULL;
        free_head_s->s_prev =  (semd_t *)ENULL;
        cur_sem->s_semAdd = semAdd;
        // todo insert before loop or after is also ok
        insertProc(&(cur_sem->s_link),p);
        setSem(semAdd,p);
        return FALSE;
    }
    else{
        semd_t* prev = (semd_t *)ENULL;
        semd_t* walker = cur_sem;
        while(walker != (semd_t *)ENULL && walker->s_semAdd != semAdd ){
             prev = walker;
             walker = walker->s_next;
        }
        // if not found we need to create one
        if (walker == (semd_t *)ENULL && free_head_s == (semd_t *)ENULL){
                return TRUE;
        }
        if (walker == (semd_t *)ENULL){
            semd_t * new_sem = free_head_s;
            free_head_s = free_head_s->s_next;
            new_sem ->s_next = (semd_t *)ENULL;
            new_sem->s_semAdd = semAdd;
            prev->s_next = new_sem;
            new_sem->s_prev = prev;
            if (free_head_s != (semd_t *)ENULL){
                free_head_s->s_prev = (semd_t *)ENULL;
            }
            insertProc(&(new_sem->s_link),p);
            setSem(semAdd,p);
            return FALSE;
        }else{
            insertProc(& walker->s_link, p);
            setSem(semAdd,p);
            return FALSE;
        }

    }
}

int initSemd(){
    int i;
    for(i = 0; i < MAXPROC; i++){
        semd_table[i].s_next = (semd_t *) ENULL;
        semd_table[i].s_prev = (semd_t *) ENULL;
        semd_table[i].s_link.next = (proc_t*)ENULL;
        semd_table[i].s_link.index = ENULL;
        semd_table[i].s_semAdd = (int*)ENULL;
    }
    for(i = 1; i<MAXPROC-1; i++){
        semd_table[i].s_next = &semd_table[i+1];
        semd_table[i].s_prev = &semd_table[i-1];
    }
    semd_table[0].s_next = &semd_table[1];
    semd_table[19].s_prev = &semd_table[18];
    cur_sem = (semd_t *) ENULL;
    free_head_s = &(semd_table[0]);
    return 0;
}
proc_t* removeBlocked(int *semAdd){

    semd_t* walker = cur_sem;
    while(walker != (semd_t *) ENULL && walker->s_semAdd != semAdd  ){
        walker = walker->s_next;
    }
    if (walker == (semd_t *) ENULL){
        return (proc_t *)ENULL;
    }
    proc_t* rv = removeProc(&(walker->s_link));
    setSem2(semAdd,rv);
    // to check if it empty the queue
    proc_t* associate_queue = headBlocked(semAdd);
    if (associate_queue == (proc_t*) ENULL){
        removeBlockHelper(walker);
    }
    return rv;
}

proc_t* outBlocked(proc_t *p){
    proc_t* rv = (proc_t*)ENULL;
    int i;
    for (i=0;i<SEMMAX;i++){
        if (p->semvec[i] != (int*) ENULL){
            semd_t* walker = cur_sem;
            while(walker != (semd_t *) ENULL && walker->s_semAdd != p->semvec[i]){
                walker = walker->s_next;
            }
            if (walker == (semd_t *) ENULL ){
                continue;
            }else{
                p->semvec[i] = (int *)ENULL;
                rv = outProc(&(walker->s_link), p);
                proc_t* associate_queue = headBlocked(walker->s_semAdd);
                if (associate_queue == (proc_t *) ENULL){
                    removeBlockHelper(walker);
                }
            }
        }
    }
    return rv;
}
proc_t* headBlocked(int *semAdd){
    if (cur_sem == (semd_t *) ENULL){
        return (proc_t *) ENULL;
    }else{
        semd_t* walker = cur_sem;
        while(walker != (semd_t *) ENULL && walker->s_semAdd != semAdd  ){
                walker = walker->s_next;
        }
        if (walker != (semd_t *) ENULL){
            return headQueue(walker->s_link);
        }
        return (proc_t *)ENULL;
    }
}
int headASL(){
    if (cur_sem == (semd_t*) ENULL){
        return FALSE;
    }
    return TRUE;
}