// This code is my own work, it was written without
// consulting a tutor or code written by other students-Wenkai Zheng
#include "../../h/vpop.h"
#include "../../h/util.h"
#include "../../h/const.h"
#include "../../h/support.h"
#include "../../h/main.e"
#include "../../h/procq.e"
#include "../../h/asl.e"
#include "../../h/trap.e"
#include "../../h/int.e"
#include "../../h/page.e"
#include "../../h/support.e"

register int r2 asm("%d2");
register int r3 asm("%d3");
register int r4 asm("%d4");

void Read_from_Terminal(state_t* old_state,int num){
     char* buf = (char*) old_state->s_r[3];
     char* content = mm[num].buf;
     // using dev arr as extern
     dev_arr[num]->d_badd = content;
     dev_arr[num]->d_op = IOREAD;
     r4 = num;
     SYS8();
     if (dev_arr[num]->d_stat == NORMAL || dev_arr[num]->d_stat == ENDOFINPUT){
         if (dev_arr[num]->d_amnt != 0){
             int i;
             for (i = 0; i<dev_arr[num]->d_amnt; i++ ){
                 buf[i] = mm[num].buf[i];
             }
             old_state->s_r[2] = dev_arr[num]->d_amnt;
         }else{
             old_state->s_r[2] = -ENDOFINPUT;
         }
     }else{
         old_state->s_r[2] = 0 - dev_arr[num]->d_stat;
     }
}

void Write_to_Terminal(state_t* old_state,int num){
    char* buf = (char*) old_state->s_r[3];
    char* content = mm[num].buf;
    int i;
    for ( i = 0;i<old_state->s_r[4]; i++){
        mm[num].buf[i] = buf[i];
    }
    dev_arr[num]->d_badd = content;
    dev_arr[num]->d_amnt = old_state->s_r[4];
    dev_arr[num]->d_op = IOWRITE;
    r4 = num;
    SYS8();
    if (dev_arr[num]->d_stat == NORMAL){
        if (dev_arr[num]->d_amnt != 0){
            old_state->s_r[2] = dev_arr[num]->d_amnt;
        }else{
            old_state->s_r[2] = -ENDOFINPUT;
        }
    }else{
        old_state->s_r[2] = 0 - dev_arr[num]->d_stat;
    }

}

void Delay(state_t* old_state, int num){
    d_sem[num].d_time = old_state->s_r[4];
    STCK(&(d_sem[num].start_time));
    vpop sem_op[2];
    sem_op[0].op = LOCK;
    sem_op[0].sem = &(d_sem[num].sem_value);
    sem_op[1].op = UNLOCK;
    sem_op[1].sem = &cron_sem;
    r3 = 2;
    r4 = (int)(&sem_op);
    SYS3();
}
void Get_Time_of_Day(state_t* old_state,int num){
    long cur_time;
    STCK(&cur_time);
    old_state->s_r[2] = cur_time;
}
void Terminate(state_t* old_state,int num){
     p_alive --;
     putframe(num);
     if (p_alive == 0){
         vpop sem_op;
         sem_op.op = UNLOCK;
         sem_op.sem = &cron_sem;
         r3 = 1;
         r4 = (int)(&sem_op);
         SYS3();
     }
     SYS2();
}