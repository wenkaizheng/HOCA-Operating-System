// This code is my own work, it was written without
// consulting a tutor or code written by other students-Wenkai Zheng
#include "../../h/vpop.h"
#include "../../h/util.h"
#include "../../h/const.h"
#include "../../h/support.h"
#include "../../h/procq.e"
#include "../../h/asl.e"
#include "../../h/main.e"
#include "../../h/int.e"
#include "../../h/trap.e"
#include "../../h/page.e"
#include "../../h/slsyscall1.e"

int boot_code[] = {
        0x41f90008,0x00002608,0x4e454a82,
        0x6c000008,0x4ef90008,0x0000d1c2,
        0x787fb882,0x6d000008,0x10bc000a,
        0x52486000,0xffde4e71
};

register int r2 asm("%d2");
register int r3 asm("%d3");
register int r4 asm("%d4");

extern int end();
extern int start();
extern int endt0();
extern int startt1();
extern int etext();
extern int startd0();
extern int endd0();
extern int startd1();
extern int edata();
extern int startb0();
extern int endb0();
extern int startb1();

static void p1a();
static void tprocess();
static void cron();
static void slproghandler();
static void slsyshandler();
static void slmmhandler();

mm_table mm[TER];
delay_sem d_sem[TER];
state_area sa[TER];
sd_t p1a_seg_table[32];
pd_t p1a_seg_table_page_table[256];
pd_t shared_seg_table_page_table[32];
int cron_sem;

static void set_page_table_entry_present(int num){
    p1a_seg_table_page_table[num].pd_r = 1;
    p1a_seg_table_page_table[num].pd_p = 1;
    p1a_seg_table_page_table[num].pd_frame = num;
}
static void set_unprivileged_seg_present(int num, int seg_num){
    mm[num].unprivileged_seg[seg_num].sd_p = 1;
    mm[num].unprivileged_seg[seg_num].sd_len = 31;
    mm[num].unprivileged_seg[seg_num].sd_prot = 7;
    if (seg_num == 1){
        mm[num].unprivileged_seg[seg_num].sd_pta = &(mm[num].unprivileged_seg_page_table[0]);
    }else{
        mm[num].unprivileged_seg[seg_num].sd_pta = &(shared_seg_table_page_table[0]);
    }
}
static void set_privileged_seg_present(int num, int seg_num){
    mm[num].privileged_seg[seg_num].sd_p = 1;
    mm[num].privileged_seg[seg_num].sd_prot = 7;
    if (seg_num == 0){
        mm[num].privileged_seg[seg_num].sd_len = 255;
    }else{
        mm[num].privileged_seg[seg_num].sd_len = 31;
    }
    if (seg_num == 2){
        mm[num].privileged_seg[seg_num].sd_pta = &(shared_seg_table_page_table[0]);
    }else if (seg_num == 0){
        mm[num].privileged_seg[seg_num].sd_pta = &(mm[num].privileged_seg_page_table[0]);
    }else if (seg_num == 1){
        mm[num].privileged_seg[seg_num].sd_pta = &(mm[num].unprivileged_seg_page_table[0]);
    }
}
static void set_privileged_page_present(int num, int page_num){
    mm[num].privileged_seg_page_table[page_num].pd_r = 1;
    mm[num].privileged_seg_page_table[page_num].pd_p = 1;
    mm[num].privileged_seg_page_table[page_num].pd_frame = page_num;
}
p1(){
    pageinit();

    // seg0 can be access by nucleus and all its resources
    p1a_seg_table[0].sd_p = 1;
    p1a_seg_table[0].sd_prot = 7;
    p1a_seg_table[0].sd_len = 255;
    p1a_seg_table[0].sd_pta = &(p1a_seg_table_page_table[0]);

    int i;
    int j;
    for(i = 1;i<32;i++){
        p1a_seg_table[i].sd_p = 0;
    }
    // page table entry for seg0
    for (i = 0; i<256;i++){
        if (i == 2){
            set_page_table_entry_present(i);
        }
        else if (i >= (int)startt1 / PAGESIZE && i<= (int)etext / PAGESIZE){
            set_page_table_entry_present(i);
        }
        else if (i>= (int)startd1 / PAGESIZE && i<= (int)edata / PAGESIZE){
            set_page_table_entry_present(i);
        }
        else if (i>=(int)startb1 / PAGESIZE && i<= (int)end / PAGESIZE){
            set_page_table_entry_present(i);
        }else if (i == Scronframe){
            set_page_table_entry_present(i);
        }else{
            // no present
            p1a_seg_table_page_table[i].pd_r = 1;
            p1a_seg_table_page_table[i].pd_p = 0;
            p1a_seg_table_page_table[i].pd_frame = i;
        }
    }
    // 2 terminal for both privileged and unprivileged seg (terminal process)
    for(i = 0;i <TER;i++){
        d_sem[i].sem_value = 0;
        d_sem[i].d_time = 0;
        d_sem[i].start_time = 0;
        // only seg0 is not accessed by terminal process
        mm[i].unprivileged_seg[0].sd_p = 0;
        // private 1
        set_unprivileged_seg_present(i,1);
        // shared 2
        set_unprivileged_seg_present(i,2);

        // 0 1 2 for privileged seg
        set_privileged_seg_present(i,0);
        set_privileged_seg_present(i,1);
        set_privileged_seg_present(i,2);

        // 3 - 31 is not present
        for(j = 3; j<32; j++){
            mm[i].unprivileged_seg[j].sd_p = 0;
            mm[i].privileged_seg[j].sd_p = 0;
        }
        // 0 -31 is not present yet for page table since not mm trap yet
        // same for the shared page table
        for (j = 0; j<32; j++){
            mm[i].unprivileged_seg_page_table[j].pd_p = 0;
            mm[i].unprivileged_seg_page_table[j].pd_r = 1;
            shared_seg_table_page_table[j].pd_p = 0;
            shared_seg_table_page_table[j].pd_r = 1;
        }
        // privileged page table need to be presented
        for (j = 0; j<256; j++){
            if (j == 2){
                set_privileged_page_present(i,j);
            }
            // device reg area
            else if (j >= 0x1400 / PAGESIZE && j<= 0x1600/PAGESIZE){
                set_privileged_page_present(i,j);
            }
            else if (j >= (int)startt1 / PAGESIZE && j<= (int)etext / PAGESIZE){
                set_privileged_page_present(i,j);
            }
            else if (j>= (int)startd1 / PAGESIZE && j<= (int)edata / PAGESIZE){
                set_privileged_page_present(i,j);
            }
            else if (j>=(int)startb1 / PAGESIZE && j<= (int)end / PAGESIZE){
                set_privileged_page_present(i,j);
            }
            else if (j == Tsysframe[i]){
                set_privileged_page_present(i,j);
            }
            else if (j == Tmmframe[i]){
                set_privileged_page_present(i,j);
            }else{
                // not present
                mm[i].privileged_seg_page_table[j].pd_r = 1;
                mm[i].privileged_seg_page_table[j].pd_p = 0;
                mm[i].privileged_seg_page_table[j].pd_frame = j;
            }
        }
    }
    // todo more var
    cron_sem = 0;
    sem_mm = 1;
    for (i = 0; i<TER;i++){
        int free_frame = getfreeframe(i,31,1);
        for (j = 0; j<11;j++){
            *(((int*)(free_frame * PAGESIZE))+j) = boot_code[j];
        }
        mm[i].unprivileged_seg_page_table[31].pd_p = 1;
        mm[i].unprivileged_seg_page_table[31].pd_frame = free_frame;
    }
    p_alive = TER;
    state_t p1a_s;
    STST(&p1a_s);
    p1a_s.s_sr.ps_s = 1;  // privileged
    p1a_s.s_sr.ps_m = 1;  // memory mapping is open
    p1a_s.s_sr.ps_int = 0; // interrupt is on
    p1a_s.s_pc = (int) p1a;
    p1a_s.s_sp = Scronstack;
    p1a_s.s_crp = &(p1a_seg_table[0]);
    // switch to p1a
    LDST(&p1a_s);
}
void static p1a(){
    state_t p1a_s;
    STST(&p1a_s);
    int i;
    for(i = 0; i<TER; i++){
        p1a_s.s_pc = (int)tprocess;
        p1a_s.s_sp = Tsysstack[i];
        p1a_s.s_crp = &(mm[i].privileged_seg[0]);
        p1a_s.s_r[4] = i;
        r4 = (int)&p1a_s;
        SYS1();
    }
    cron();
}
void static tprocess(){
    state_t tp;
    STST(&tp);
    int num = tp.s_r[4];
    // three types of traps
    STST(&sa[num].prog_new);
    sa[num].prog_new.s_pc = (int)slproghandler;
    sa[num].prog_new.s_sp = Tsysstack[num];
    sa[num].prog_new.s_crp = &(mm[num].privileged_seg[0]);
    sa[num].prog_new.s_r[4] = num;


    STST(&sa[num].mm_new);
    sa[num].mm_new.s_pc = (int)slmmhandler;
    sa[num].mm_new.s_sp = Tmmstack[num];
    sa[num].mm_new.s_crp = &(mm[num].privileged_seg[0]);
    sa[num].mm_new.s_r[4] = num;


    STST(&sa[num].sys_new);
    sa[num].sys_new.s_pc = (int)slsyshandler;
    sa[num].sys_new.s_sp = Tsysstack[num];
    sa[num].sys_new.s_crp = &(mm[num].privileged_seg[0]);
    sa[num].sys_new.s_r[4] = num;


    r2 = 0;
    r3 = (int)&(sa[num].prog_old);
    r4 = (int)&(sa[num].prog_new);
    SYS5();

    r2 = 1;
    r3 = (int)&(sa[num].mm_old);
    r4 = (int)&(sa[num].mm_new);
    SYS5();

    r2 = 2;
    r3 = (int)&(sa[num].sys_old);
    r4 = (int)&(sa[num].sys_new);
    SYS5();

   // load the boot
   tp.s_sr.ps_s = 0; // unprivileged
   tp.s_pc = 0x80000+31*PAGESIZE;
   // todo check sp
   tp.s_sp = 0x80000+32*PAGESIZE-2;
   tp.s_crp = &(mm[num].unprivileged_seg[0]);
   LDST(&tp);

}
void static slproghandler(){
    state_t slpt;
    STST(&slpt);
    int num = slpt.s_r[4];
    Terminate(&sa[num].prog_old,num);
    LDST(&sa[num].prog_old);
}
void static slmmhandler(){
    state_t slmm;
    STST(&slmm);
    int num = slmm.s_r[4];
    int page = sa[num].mm_old.s_tmp.tmp_mm.mm_pg;
    int seg = sa[num].mm_old.s_tmp.tmp_mm.mm_seg;

    if(mm[num].privileged_seg[seg].sd_p == 0){
        Terminate(&sa[num].mm_old,num);
    }else if (seg == 0){
        // seg 0 can't be accessed by t process
        Terminate(&sa[num].mm_old,num);
    }else if (seg == 1){
        int page_frame = getfreeframe(num,page,1);
        // if the reference bit is 0
        if (mm[num].unprivileged_seg_page_table[page].pd_r == 0){
            pagein(num,page,1,page_frame);
        }
        vpop sem_op;

        sem_op.op = LOCK;
        sem_op.sem = &sem_mm;
        r3 = 1;
        r4 = (int)&sem_op;
        SYS3();

        mm[num].unprivileged_seg_page_table[page].pd_frame = page_frame;
        mm[num].unprivileged_seg_page_table[page].pd_p = 1;

        sem_op.op = UNLOCK;
        sem_op.sem = &sem_mm;
        r3 = 1;
        r4 = (int)&sem_op;
        SYS3();
    }else{
        int page_frame = getfreeframe(num,page,2);
        // if the reference bit is 0
        if (shared_seg_table_page_table[page].pd_r == 0){
            pagein(num,page,2,page_frame);
        }

        vpop sem_op;

        sem_op.op = LOCK;
        sem_op.sem = &sem_mm;
        r3 = 1;
        r4 = (int)&sem_op;
        SYS3();

        shared_seg_table_page_table[page].pd_frame = page_frame;
        shared_seg_table_page_table[page].pd_p = 1;

        sem_op.op = UNLOCK;
        sem_op.sem = &sem_mm;
        r3 = 1;
        r4 = (int)&sem_op;
        SYS3();

    }
    LDST(&sa[num].mm_old);
}
void static slsyshandler(){
    state_t slsys;
    STST(&slsys);
    int num = slsys.s_r[4];
    int sys_num = sa[num].sys_old.s_tmp.tmp_sys.sys_no;
    switch (sys_num) {
        case 9:
            Read_from_Terminal(&sa[num].sys_old,num);
            break;
        case 10:
            Write_to_Terminal(&sa[num].sys_old,num);
            break;
        case 13:
            Delay(&sa[num].sys_old,num);
            break;
        case 16:
            Get_Time_of_Day(&sa[num].sys_old,num);
            break;
        case 17:
            Terminate(&sa[num].sys_old,num);
            break;
        default:
            break;
    }
    LDST(&sa[num].sys_old);
}
void static cron(){
    while( p_alive > 0){
        int i;
        for(i = 0; i<TER;i++){
            if (d_sem[i].sem_value < 0){
                long cur;
                STCK(&cur);
                if (cur - d_sem[i].start_time > d_sem[i].d_time){
                    vpop sem_op;
                    sem_op.sem = &(d_sem[i].sem_value);
                    sem_op.op = UNLOCK;
                    r3 = 1;
                    r4 = (int)&sem_op;
                    SYS3();
                }
            }
        }
        int clock = 0;
        for (i = 0; i<TER; i++){
            // clock interrupt occur
            if (d_sem[i].sem_value < 0){
                clock = 1;
            }
        }

        if (clock){
            SYS7();
        }else{
            vpop sem_op;
            sem_op.sem = &(cron_sem);
            sem_op.op = LOCK;
            r3 = 1;
            r4 = (int)&sem_op;
            SYS3();
        }
    }
    SYS2();
}