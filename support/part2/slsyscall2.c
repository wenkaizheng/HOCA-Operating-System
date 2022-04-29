// This code is my own work, it was written without
// consulting a tutor or code written by other students-Wenkai Zheng
#include "../../h/vpop.h"
#include "../../h/util.h"
#include "../../h/const.h"
#include "../../h/support.h"
#include "../part1/h/tconst.h"
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

void P_Virtual_Semaphore(state_t* old_state, int num){
    P(&protected_sem);
    int* vpop_addr = (int*) (old_state->s_r[4]);
    if (vpop_addr >= (int*)SEG2){
        // check large than 0
         if (*vpop_addr > 0){
             *vpop_addr -= 1;
             V(&protected_sem);
         }else{
             int i;
             for (i = 0;i<MAXPROC;i++){
                 if(vs_arr[i].addr == 0){

                     vs_arr[i].addr = vpop_addr;
                     *vpop_addr -= 1;
                     V(&protected_sem);

                     vpop sem_op;
                     sem_op.sem = &(vs_arr[i].sem_value);
                     sem_op.op = LOCK;
                     r3 = 1;
                     r4 = (int)&sem_op;
                     SYS3();
                     break;
                 }
             }
         }
    }else{
        V(&protected_sem);
        Terminate(old_state,num);
    }
}

void V_Virtual_Semaphore(state_t* old_state, int num){
    P(&protected_sem);
    int* vpop_addr = (int*) (old_state->s_r[4]);
    if (vpop_addr >= (int*)SEG2){

        // check large than 0
        if (*vpop_addr >= 0){
            *vpop_addr += 1;
            V(&protected_sem);
        }else{
            int i;
            for (i = 0;i<MAXPROC;i++){
                if(vs_arr[i].addr == vpop_addr){

                    vs_arr[i].addr = 0;
                    *vpop_addr += 1;
                    V(&protected_sem);

                    vpop sem_op;
                    sem_op.sem = &(vs_arr[i].sem_value);
                    sem_op.op = UNLOCK;
                    r3 = 1;
                    r4 = (int)&sem_op;
                    SYS3();
                    break;
                }
            }
        }
        //
    }else{
        V(&protected_sem);
        Terminate(old_state,num);
    }
}

void Disk_Put(state_t* old_state,int num){
    char* buf = (char*) old_state->s_r[3];
    if ((int*)buf < (int*)SEG1){
        Terminate(old_state,num);
    }
    int i;
    for(i = 0;i<512;i++){
        mm[num].buf[i] = buf[i];
    }
    ds_arr[num].track = old_state->s_r[4];
    ds_arr[num].sector = old_state->s_r[2];
    ds_arr[num].op = IOWRITE;

    vpop sem_op[2];
    sem_op[0].sem = &(ds_arr[num].sem_value);
    sem_op[0].op = LOCK;
    sem_op[1].sem = &sem_disk;
    sem_op[1].op = UNLOCK;
    r3 = 2;
    r4 = (int)&sem_op;
    SYS3();
    if(dev_arr[7]->d_stat == NORMAL){
       old_state->s_r[2] = 512;
    }else{
        old_state->s_r[2] = -dev_arr[7]->d_stat;
    }

}
void Disk_Get(state_t* old_state,int num){
    char* buf = (char*) old_state->s_r[3];
    if ((int*)buf < (int*)SEG1){
        Terminate(old_state,num);
    }
    ds_arr[num].track = old_state->s_r[4];
    ds_arr[num].sector = old_state->s_r[2];
    ds_arr[num].op = IOREAD;

    vpop sem_op[2];
    sem_op[0].sem = &(ds_arr[num].sem_value);
    sem_op[0].op = LOCK;
    sem_op[1].sem = &sem_disk;
    sem_op[1].op = UNLOCK;
    r3 = 2;
    r4 = (int)&sem_op;
    SYS3();
    if(dev_arr[7]->d_stat == NORMAL){
        int i;
        for(i = 0;i<512;i++){
            buf[i] = mm[num].buf[i];
        }
        old_state->s_r[2] = 512;
    }else{
        old_state->s_r[2] = -dev_arr[7]->d_stat;
    }

}
int tasker_helper(int track,int sector, int dir){
    int rv = -1;
    int min = 10000;
    if (dir == 0){
        int i;
        for(i = 0; i< MAXPROC;i++){
            if(ds_arr[i].sem_value == -1){
                int value = (ds_arr[i].track - track) * 8 + ds_arr[i].sector - sector;
                if (value >=0 && value < min){
                    min = value;
                    rv = i;
                }
            }
        }
    }else{
        int i;
        for(i = 0; i< MAXPROC;i++){
            if(ds_arr[i].sem_value == -1){
                int value = (track - ds_arr[i].track) * 8 + sector - ds_arr[i].sector;
                if (value>=0 && value < min){
                    min = value;
                    rv = i;
                }
            }
        }
    }
    return rv;
}
void diskdaemon(){
    int track = 0;
    int sector = 0;
    int dir = 0;

    while(p_alive > 0){
        vpop sem_op;
        sem_op.sem = &sem_disk;
        sem_op.op = LOCK;
        r3 = 1;
        r4 = (int)&sem_op;
        SYS3();
        int num = tasker_helper(track,sector,dir);
        if (num == -1){
            dir ^= 1;
            num = tasker_helper(track,sector,dir);
        }

        track = ds_arr[num].track;
        sector = ds_arr[num].sector;
        dev_arr[7]->d_track = track;
        dev_arr[7]->d_op = IOSEEK;
        r4 = 7;
        SYS8();
        // if seek is not success, we won't do other op
        if(dev_arr[7]->d_stat == NORMAL) {
            dev_arr[7]->d_sect = sector;
            dev_arr[7]->d_badd = mm[num].buf;
            dev_arr[7]->d_op = ds_arr[num].op;
            r4 = 7;
            SYS8();
        }
        sem_op.sem = &(ds_arr[num].sem_value);
        sem_op.op = UNLOCK;
        r3 = 1;
        r4 = (int)&sem_op;
        SYS3();
    }
    SYS2();
}
