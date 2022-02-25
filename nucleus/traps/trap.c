// This code is my own work, it was written without
// consulting a tutor or code written by other students-Wenkai Zheng
#include "../../h/const.h"
#include "../../h/types.h"
#include "../../h/vpop.h"
#include "../../h/util.h"
#include "../../h/procq.e"
#include "../../h/asl.e"
#include "../../h/syscall.e"
#include "../../h/main.e"
#include "../../h/int.e"

static state_t* old_sys_call_state;
static state_t* old_mm_state;
static state_t* old_pro_state;

void start_time(proc_t* p){

    STCK(&(p->start_time));
}
void get_interval(proc_t* p){
    long cur_time;
    STCK(&cur_time);
    p->cpu_time += cur_time - p->start_time;
}
void pass_up(state_t* old_state, int type){
    proc_t* cur = headQueue(running_queue);
    if (type == 0){
        if(cur->prog_new != (state_t*)ENULL){
            *cur->prog_old = *old_state;
            start_time(cur);
            LDST(cur->prog_new);
        }else{
            Terminate_Process(old_state);
        }
    }else if (type == 1){
        if(cur->memory_management_new != (state_t*)ENULL){
            *cur->memory_management_old = *old_state;
            start_time(cur);
            LDST(cur->memory_management_new);
        }else{
            Terminate_Process(old_state);
        }
    }else if (type == 2){
         if (cur->system_call_new != (state_t*)ENULL){
             *cur->system_call_old = *old_state;
             start_time(cur);
             LDST(cur->system_call_new);
         }
         else{
            Terminate_Process(old_state);
         }
    }
}
void static trapmmhandler(){
    proc_t* cur = headQueue(running_queue);
    get_interval(cur);
    pass_up(old_mm_state,1);
}
void static trapproghandler(){
    proc_t* cur = headQueue(running_queue);
    get_interval(cur);
    pass_up(old_pro_state,0);
}
void static trapsyshandler(){
    proc_t* cur = headQueue(running_queue);
    get_interval(cur);
    // check the sys number and mode from old sys state
    if (old_sys_call_state->s_sr.ps_s != 1 && old_sys_call_state->s_tmp.tmp_sys.sys_no >= 1 &&
      old_sys_call_state->s_tmp.tmp_sys.sys_no <= 8){
        old_sys_call_state->s_tmp.tmp_pr.pr_typ = PRIVILEGE;
        pass_up(old_sys_call_state,0);
    }
    int sys_no = old_sys_call_state->s_tmp.tmp_sys.sys_no;
    switch (sys_no) {
        case 1:
            Create_Process(old_sys_call_state);
            break;
        case 2:
            // todo not para
            Terminate_Process();
            break;
        case 3:
            Sem_OP(old_sys_call_state);
            break;
        case 4:
            notused();
            break;
        case 5:
            Specify_Trap_State_Vector(old_sys_call_state);
            break;
        case 6:
            Get_CPU_Time(old_sys_call_state);
            break;
        case 7:
            waitforpclock();
            break;
        case 8:
            waitforio();
            break;
        default:
            // check the sys call
            pass_up(old_sys_call_state,2);
            break;

    }
    start_time(cur);
    LDST(old_sys_call_state);
}
void trapinit(){
    *(int *)0x008 = (int)STLDMM;
    *(int *)0x00c = (int)STLDADDRESS;
    *(int *)0x010 = (int)STLDILLEGAL;
    *(int *)0x014 = (int)STLDZERO;
    *(int *)0x020 = (int)STLDPRIVILEGE;
    *(int *)0x08c = (int)STLDSYS;
    *(int *)0x94 = (int)STLDSYS9;
    *(int *)0x98 = (int)STLDSYS10;
    *(int *)0x9c = (int)STLDSYS11;
    *(int *)0xa0 = (int)STLDSYS12;
    *(int *)0xa4 = (int)STLDSYS13;
    *(int *)0xa8 = (int)STLDSYS14;
    *(int *)0xac = (int)STLDSYS15;
    *(int *)0xb0 = (int)STLDSYS16;
    *(int *)0xb4 = (int)STLDSYS17;
    *(int *)0x100 = (int)STLDTERM0;
    *(int *)0x104 = (int)STLDTERM1;
    *(int *)0x108 = (int)STLDTERM2;
    *(int *)0x10c = (int)STLDTERM3;
    *(int *)0x110 = (int)STLDTERM4;
    *(int *)0x114 = (int)STLDPRINT0;
    *(int *)0x11c = (int)STLDDISK0;
    *(int *)0x12c = (int)STLDFLOPPY0;
    *(int *)0x140 = (int)STLDCLOCK;

    old_pro_state = (state_t *)0x800;    // old prog area
    state_t* new_pro_state = (state_t *)0x800+1;   // + 76 bytes to next area // (state*) (0x800 + 76)
    new_pro_state->s_pc = (int)trapproghandler;   // pc point to handler
    new_pro_state->s_sp = init_state.s_sp;          // init is the first state from STST
    new_pro_state->s_sr.ps_s = 1;            //  privilege mode
    new_pro_state->s_sr.ps_m = 0;            // memory mapping to 1
    new_pro_state->s_sr.ps_int = 7;         // disable interrupt

    old_mm_state = (state_t *)0x898;
    state_t* new_mm_state = (state_t *)0x898 + 1;
    new_mm_state->s_pc = (int)trapmmhandler;
    new_mm_state->s_sp = init_state.s_sp;
    new_mm_state->s_sr.ps_s = 1;
    new_mm_state->s_sr.ps_m = 0;
    new_mm_state->s_sr.ps_int = 7;

    old_sys_call_state = (state_t *)0x930;
    state_t* new_sys_call_state = (state_t *)0x930 + 1;
    new_sys_call_state->s_pc = (int)trapsyshandler;
    new_sys_call_state->s_sp = init_state.s_sp;
    new_sys_call_state->s_sr.ps_s = 1;
    new_sys_call_state->s_sr.ps_m = 0;
    new_sys_call_state->s_sr.ps_int = 7;
}