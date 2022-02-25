// This code is my own work, it was written without
// consulting a tutor or code written by other students-Wenkai Zheng
#include "../../h/const.h"
#include "../../h/types.h"
#include "../../h/vpop.h"
#include "../../h/util.h"
#include "../../h/int.e"
#include "../../h/trap.e"
#include "../../h/asl.e"
#include "../../h/procq.e"

extern  int p1();
state_t init_state;
proc_link  running_queue;

void static init(){
    running_queue.next = (proc_t*)ENULL;
    running_queue.index = ENULL;
    STST(&init_state);
    initProc();
    initSemd();
    trapinit();
    intinit();
}
void schedule(){
    proc_t* cur = headQueue(running_queue);
    if (cur == (proc_t*)ENULL){
        intdeadlock();
    }else{
        intschedule();
        cur =  headQueue(running_queue);
        start_time(cur);
        LDST(&(cur->p_s));
    }
}
void main(){
    init();
    proc_t* first_process = allocProc();
    STST(&(first_process->p_s));
    // 2 pages for stack
    first_process->p_s.s_sp  = init_state.s_sp - 512 * 2;
    first_process->p_s.s_pc = (int)p1;
    insertProc(&running_queue,first_process);
    schedule();
}