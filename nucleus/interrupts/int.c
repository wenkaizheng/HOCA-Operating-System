// This code is my own work, it was written without
// consulting a tutor or code written by other students-Wenkai Zheng
#include <string.h>
#include "../../h/const.h"
#include "../../h/types.h"
#include "../../h/vpop.h"

#include "../../h/procq.e"
#include "../../h/asl.e"
#include "../../h/int.e"
#include "../../h/trap.e"
#include "../../h/main.e"
#define  TERMINAL 0
#define  PRINTER 1
#define  DISK   2
#define  FLOPPY 3
state_t* old_printer_state;
state_t* old_terminal_state;
state_t* old_disk_state;
state_t* old_floppy_state;
state_t* old_clock_state;
int dev_sem[15];
devreg_t* dev_arr[15];
int clock_lock;
typedef struct saved_info{
    int amnt;
    int stat;
}saved_info;
saved_info save_arr[15];
int clock_interrupt_time = 100000;
int quantum = 50000;
int clock_count = 0;
void static intsemop(int* sem_value, int op);
void static intterminalhandler();
void static intprinterhandler();
void static intdiskhandler();
void static intfloppyhandler();
void static intclockhandler();
void intschedule()
{
    LDIT(&quantum);
}

void static sleep(){
    asm("stop   #0X2000");
}

void intinit()
{
    clock_lock = 0;
    int i;
    for(i = 0;i<15;i++){
        dev_sem[i] = 0;
        dev_arr[i] = (devreg_t*)(0x1400 + i * 0x10);
    }
    old_terminal_state = (state_t*)0x9c8;
    state_t* new_terminal_state = (state_t *)0x9c8+1;
    new_terminal_state->s_pc = (int) intterminalhandler;   // pc point to handler
    new_terminal_state->s_sp = init_state.s_sp;          // init is the first state from STST
    new_terminal_state->s_sr.ps_s = 1;            //  privilege mode
    new_terminal_state->s_sr.ps_m = 0;            // memory mapping to 1
    new_terminal_state->s_sr.ps_int = 7;         // disable interrupt


    old_printer_state = (state_t*)0xa60;
    state_t* new_printer_state = (state_t *)0xa60+1;
    new_printer_state->s_pc = (int) intprinterhandler;
    new_printer_state->s_sp = init_state.s_sp;
    new_printer_state->s_sr.ps_s = 1;
    new_printer_state->s_sr.ps_m = 0;
    new_printer_state->s_sr.ps_int = 7;


    old_disk_state = (state_t*)0xaf8;
    state_t* new_disk_state = (state_t *)0xaf8+1;
    new_disk_state->s_pc = (int) intdiskhandler;
    new_disk_state->s_sp = init_state.s_sp;
    new_disk_state->s_sr.ps_s = 1;
    new_disk_state->s_sr.ps_m = 0;
    new_disk_state->s_sr.ps_int = 7;


    old_floppy_state = (state_t*)0xb90;
    state_t* new_floppy_state = (state_t *)0x9c8+1;
    new_floppy_state->s_pc = (int) intfloppyhandler;
    new_floppy_state->s_sp = init_state.s_sp;
    new_floppy_state->s_sr.ps_s = 1;
    new_floppy_state->s_sr.ps_m = 0;
    new_floppy_state->s_sr.ps_int = 7;


    old_clock_state = (state_t*)0xc28;
    state_t* new_clock_state = (state_t *)0xc28+1;
    new_clock_state->s_pc = (int) intclockhandler;
    new_clock_state->s_sp = init_state.s_sp;
    new_clock_state->s_sr.ps_s = 1;
    new_clock_state->s_sr.ps_m = 0;
    new_clock_state->s_sr.ps_int = 7;
}

void waitforpclock(state_t *old){
    proc_t* head = headQueue(running_queue);
    head->p_s = *old;
    intsemop(&clock_lock,LOCK);
}

void waitforio(state_t *old){
    int dev = old->s_r[4];
    if (dev_sem[dev] == 1){
        // interrupt has already occurs store the value back
        dev_sem[dev] -=1;
        old->s_r[2] = save_arr[dev].amnt;
        old->s_r[3] = save_arr[dev].stat;
    }else{
        // interrupt is not occur yet
        proc_t* head = headQueue(running_queue);
        head->p_s = *old;
        intsemop(&dev_sem[dev],LOCK);
    }

}

void static intsemop(int* sem_value, int op){
    if (op == LOCK){
        if (--(*(sem_value)) < 0){
            proc_t* removed = removeProc(&running_queue);
            if(removed != (proc_t*)ENULL){
                insertBlocked(sem_value,removed);
            }
            schedule();
        }
    }else{
        if ((*(sem_value))++ <0){
            proc_t* removed = removeBlocked(sem_value);
            if (removed != (proc_t*)ENULL && removed->qcount == 0){
                insertProc(&running_queue,removed);
            }
        }
    }
}
void static inthandler(int type, int dev){
    int dev_pos = -1;
    if (type == TERMINAL){
        dev_pos = 0 + dev;
    }else if (type == PRINTER){
        dev_pos = 5 + dev;
    }else if (type == DISK){
        dev_pos = 7 + dev;
    }else if (type == FLOPPY){
        dev_pos = 11 + dev;
    }
    if (dev_sem[dev_pos] == -1){
        // interrupt occur after wait io device
        proc_t* block_dev = headBlocked(&dev_sem[dev_pos]);
        block_dev->p_s.s_r[2] = dev_arr[dev_pos]->d_amnt;
        block_dev->p_s.s_r[3] = dev_arr[dev_pos]->d_stat;
        intsemop(&dev_sem[dev_pos],UNLOCK);
    }else{
        // interrupt occur before wait io device become 1
        dev_sem[dev_pos] += 1;
        save_arr[dev_pos].amnt = dev_arr[dev_pos]->d_amnt;
        save_arr[dev_pos].stat = dev_arr[dev_pos]->d_stat;
    }
}
void static intterminalhandler(){
    proc_t* head = headQueue(running_queue);
    inthandler(TERMINAL,old_terminal_state->s_tmp.tmp_int.in_dno);
    if (head != (proc_t*)ENULL){
        LDST(old_terminal_state);
    }else{
        schedule();
    }
}
void static intprinterhandler(){
    proc_t* head = headQueue(running_queue);
    inthandler(PRINTER,old_printer_state->s_tmp.tmp_int.in_dno);
    if (head != (proc_t*)ENULL){
        LDST(old_printer_state);
    }else{
        schedule();
    }
}
void static intdiskhandler(){
    proc_t* head = headQueue(running_queue);
    inthandler(DISK,old_disk_state->s_tmp.tmp_int.in_dno);
    if (head != (proc_t*)ENULL){
        LDST(old_disk_state);
    }else{
        schedule();
    }
}
void static intfloppyhandler(){
    proc_t* head = headQueue(running_queue);
    inthandler(FLOPPY,old_floppy_state->s_tmp.tmp_int.in_dno);
    if (head != (proc_t*)ENULL){
        LDST(old_floppy_state);
    }else{
        schedule();
    }
}
void static intclockhandler(){
    proc_t* head = headQueue(running_queue);
    clock_count += quantum;

    if (head != (proc_t*)ENULL){
        // remove and insert
        proc_t* removed = removeProc(&running_queue);
        get_interval(removed);
        removed->p_s = *old_clock_state;
        insertProc(&running_queue,removed);
    }
    // v every 100 milliseconds.
    if (clock_count >= clock_interrupt_time){
        clock_count =  0;
        if (headBlocked(&clock_lock) != (proc_t*)ENULL){
                intsemop(&clock_lock,UNLOCK);
        }
    }
    schedule();
}
void printer(int status){
    if (status == 0){
        // not normal
        char a[100] = "nucleus: normal termination";
        dev_arr[5]->d_stat = -1;
        dev_arr[5]->d_badd = a;
        dev_arr[5]->d_amnt = strlen(a);
        dev_arr[5]->d_op = IOWRITE;
        while(dev_arr[5]->d_stat != NORMAL);

    }else{
        char a[100] = "nucleus: deadlock termination";
        dev_arr[5]->d_stat = -1;
        dev_arr[5]->d_op = -1;
        dev_arr[5]->d_badd = a;
        dev_arr[5]->d_amnt = strlen(a);
        dev_arr[5]->d_op = IOWRITE;
        while(dev_arr[5]->d_stat != NORMAL);
    }
}
void intdeadlock()
{
    int i;
    int dev_block = 0;
    for (i = 0; i<15; i++){
      if(headBlocked(&dev_sem[i]) != (proc_t*) ENULL){
          dev_block = 1;
      }
    }
    if (headBlocked(&clock_lock) != (proc_t*) ENULL){
        intschedule();
        sleep();
    }else if (dev_block){
        sleep();
    }else if (headASL() == FALSE){
        // check no one left in asl
        printer(0);
        HALT();
    }else{
        printer(1);
        HALT();
    }
}