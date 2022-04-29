// This code is my own work, it was written without
// consulting a tutor or code written by other students-Wenkai Zheng
#include "../../h/const.h"
#include "../../h/util.h"
#include "../../h/vpop.h"
#include "../../h/support.h"
#include "../../h/procq.e"
#include "../../h/asl.e"
#include "../../h/main.e"
#include "../../h/int.e"
#include "../../h/trap.e"
#include "../../h/page.e"
#include "../../h/support.e"
#include "../../h/slsyscall1.e"
#include "../../h/slsyscall2.e"

#define MAXFRAMES 20
#define HIGH 4
#define LOW 2
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

int sem_mm;
int sem_floppy;
int sem_disk;
int sem_page;
int p_alive;
int protected_sem;

int sem_pf;
int pf_ctr, pf_start;
int Tsysframe[5];
int Tmmframe[5];
int Scronframe, Spagedframe, Sdiskframe;
int Tsysstack[5];
int Tmmstack[5];
int Scronstack, Spagedstack, Sdiskstack;
int disk[100][8];

sd_t page_daemon_seg[32];
sd_t disk_daemon_seg[32];
pd_t page_daemon_page[256];
pd_t disk_daemon_page[256];

diks_scheduler  ds_arr[MAXPROC];
page_frame_scheduler pfs_arr[MAXFRAMES];
virtual_sem vs_arr[MAXPROC];
int disk[100][8];
static void pagedaemon();

void P(int* sem_value){
     vpop sem_op;
     sem_op.sem = sem_value;
     sem_op.op = LOCK;
     r3 = 1;
     r4 = (int)&sem_op;
     SYS3();
}

void V(int* sem_value){
    vpop sem_op;
    sem_op.sem = sem_value;
    sem_op.op = UNLOCK;
    r3 = 1;
    r4 = (int)&sem_op;
    SYS3();
}
static int get_disk_pos(){
    int i;
    int j;
    for (i = 0;i<100;i++){
        for(j = 0;j<8;j++){
            if(disk[i][j] == -1){
                disk[i][j] = 1;
                return (i << 3) | j;
            }
        }
    }
}
void set_page_present(int num, int judge){
    if (judge == 0){
        page_daemon_page[num].pd_r = 1;
        page_daemon_page[num].pd_p = 1;
        page_daemon_page[num].pd_frame = num;
        disk_daemon_page[num].pd_r = 1;
        disk_daemon_page[num].pd_p = 1;
        disk_daemon_page[num].pd_frame = num;
    }else if (judge == 1){
        // disk
        disk_daemon_page[num].pd_r = 1;
        disk_daemon_page[num].pd_p = 1;
        disk_daemon_page[num].pd_frame = num;
    }else{
        //page
        page_daemon_page[num].pd_r = 1;
        page_daemon_page[num].pd_p = 1;
        page_daemon_page[num].pd_frame = num;
    }
}
void page_in_out_helper(int track,int sector,int op, int pf){
    vpop sem_op[2];
    sem_op[0].sem = &sem_mm;
    sem_op[0].op = UNLOCK;
    sem_op[1].sem = &sem_floppy;
    sem_op[1].op = LOCK;
    r3 = 2;
    r4 = (int)&sem_op;
    SYS3();

    dev_arr[11]->d_track = track;
    dev_arr[11]->d_op = IOSEEK;
    r4 = 11;
    SYS8();
    dev_arr[11]->d_badd = (char*)(pf * PAGESIZE);
    dev_arr[11]->d_sect = sector;
    dev_arr[11]->d_op = op;
    r4 = 11;
    SYS8();

    sem_op[0].sem = &sem_mm;
    sem_op[0].op = LOCK;
    sem_op[1].sem = &sem_floppy;
    sem_op[1].op = UNLOCK;
    r3 = 2;
    r4 = (int)&sem_op;
    SYS3();
}
pageinit()
{
  int endframe;

  /* check if you have space for 35 page frames, the system
     has 128K */
  endframe=(int)end / PAGESIZE;
  if (endframe > 220 ) { /* 110 K */
    HALT();
  }

  Tsysframe[0] = endframe + 2;
  Tsysframe[1] = endframe + 3;
  Tsysframe[2] = endframe + 4;
  Tsysframe[3] = endframe + 5;
  Tsysframe[4] = endframe + 6;
  Tmmframe[0]  = endframe + 7;
  Tmmframe[1]  = endframe + 8;
  Tmmframe[2]  = endframe + 9;
  Tmmframe[3]  = endframe + 10;
  Tmmframe[4]  = endframe + 11;
  Scronframe  = endframe + 12;
  Spagedframe = endframe + 13;
  Sdiskframe  = endframe + 14;

  Tsysstack[0] = (endframe + 3)*512 - 2;
  Tsysstack[1] = (endframe + 4)*512 - 2;
  Tsysstack[2] = (endframe + 5)*512 - 2;
  Tsysstack[3] = (endframe + 6)*512 - 2;
  Tsysstack[4] = (endframe + 7)*512 - 2;
  Tmmstack[0]  = (endframe + 8)*512 - 2;
  Tmmstack[1]  = (endframe + 9)*512 - 2;
  Tmmstack[2]  = (endframe + 10)*512 - 2;
  Tmmstack[3]  = (endframe + 11)*512 - 2;
  Tmmstack[4]  = (endframe + 12)*512 - 2;
  Scronstack   = (endframe + 13)*512 - 2;
  Spagedstack  = (endframe + 14)*512 - 2;
  Sdiskstack   = (endframe + 15)*512 - 2;
  pf_start    = (endframe + 17);

  /*
  pf_start = (int)end / PAGESIZE + 1; 
*/
  sem_pf = MAXFRAMES;
  pf_ctr = 0;
  sem_disk = 0;
  sem_page = 0;
  sem_floppy = 1;
  protected_sem = 1;

  int i;
  int j;
  for (i = 0;i<MAXPROC;i++){
      ds_arr[i].sem_value = 0;
      ds_arr[i].op = -1;
      ds_arr[i].track = -1;
      ds_arr[i].sector = -1;

      vs_arr[i].addr = 0;
      vs_arr[i].sem_value = 0;
  }
  for(i = 0;i<MAXFRAMES;i++){
      pfs_arr[i].pid = -1;
      pfs_arr[i].seg = -1;
      pfs_arr[i].page = -1;
      pfs_arr[i].track = -1;
      pfs_arr[i].sector = -1;
  }
  for (i = 0;i<100;i++){
      for(j = 0; j<8;j++){
          disk[i][j] = -1;
      }
  }
  page_daemon_seg[0].sd_p = 1;
  page_daemon_seg[0].sd_len = 255;
  page_daemon_seg[0].sd_prot = 7;
  page_daemon_seg[0].sd_pta = &(page_daemon_page[0]);

  disk_daemon_seg[0].sd_p = 1;
  disk_daemon_seg[0].sd_len = 255;
  disk_daemon_seg[0].sd_prot = 7;
  disk_daemon_seg[0].sd_pta = &(disk_daemon_page[0]);

  for(i = 1; i<32; i++){
      page_daemon_seg[i].sd_p = 0;
      disk_daemon_seg[i].sd_p = 0;
  }
  for(i = 0;i<256;i++){
      if (i == 2){
          set_page_present(i,0);
      }else if (i >= 0x1400/PAGESIZE && i<=0x1600/PAGESIZE){
          set_page_present(i,0);
      }else if (i >= (int)startt1 / PAGESIZE && i<= (int)etext / PAGESIZE){
          set_page_present(i,0);
      }else if (i>= (int)startd1 / PAGESIZE && i<= (int)edata / PAGESIZE){
          set_page_present(i,0);
      }else if (i>=(int)startb1 / PAGESIZE && i<= (int)end / PAGESIZE){
          set_page_present(i,0);
      }else if (i == Sdiskframe){
          set_page_present(i,1);
      }else if (i == Spagedframe){
          set_page_present(i,2);
      }else{
          page_daemon_page[i].pd_r = 1;
          page_daemon_page[i].pd_p = 0;
          page_daemon_page[i].pd_frame = i;
          disk_daemon_page[i].pd_r = 1;
          disk_daemon_page[i].pd_p = 0;
          disk_daemon_page[i].pd_frame = i;
      }
  }
  state_t page_daemon;
  STST(&page_daemon);
  page_daemon.s_sr.ps_s = 1;
  page_daemon.s_sr.ps_m = 1;
  page_daemon.s_sr.ps_int = 0;
  page_daemon.s_pc = (int)pagedaemon;
  page_daemon.s_sp = Spagedstack;
  page_daemon.s_crp = &(page_daemon_seg[0]);
  r4 = (int)&page_daemon;
  SYS1();

  state_t disk_daemon;
  STST(&disk_daemon);
  disk_daemon.s_sr.ps_s = 1;
  disk_daemon.s_sr.ps_m = 1;
  disk_daemon.s_sr.ps_int = 0;
  disk_daemon.s_pc = (int)diskdaemon;
  disk_daemon.s_sp = Sdiskstack;
  disk_daemon.s_crp = &(disk_daemon_seg[0]);
  r4 = (int)&disk_daemon;
  SYS1();
}

getfreeframe(term,page,seg)
int term, page, seg;
{
    P(&sem_pf);
    P(&sem_mm);
    // check the seg is sharded or not

    int shared = 0;
    int rv = -1;
    if (seg == 2){
        int i;
        for(i = 0; i<MAXFRAMES; i++){
            if (pfs_arr[i].pid == -1){
                continue;
            }else{
                if (pfs_arr[i].page == page && pfs_arr[i].seg == seg){
                    // it is a shard memory we can use this frame
                    shared = 1;
                    rv = pf_start + i;
                    break;
                }
            }
        }
    }

    if (!shared){
        int i;
        for(i = 0;i<MAXFRAMES;i++){
            if(pfs_arr[i].pid == -1){
                pfs_arr[i].pid = term;
                pfs_arr[i].page = page;
                pfs_arr[i].seg = seg;
                pfs_arr[i].track = -1;
                pfs_arr[i].sector = -1;
                rv = pf_start + i;
                break;
            }
        }
   }
    if (sem_pf <= LOW){
        V(&sem_page);
    }
    V(&sem_mm);
    return rv;
}

pagein(term,page,seg,pf)
int term, page, seg, pf;
{
    P(&sem_mm);
    int page_frame = -1;
   if (seg == 1){
       page_frame = mm[term].unprivileged_seg_page_table[page].pd_frame;
   }else if (seg == 2){
       page_frame = shared_seg_table_page_table[page].pd_frame;
   }

   int track = page_frame >> 3;
   int sector = page_frame & 7;

   page_in_out_helper(track,sector,IOREAD,pf);

   pfs_arr[pf-pf_start].track = track;
   pfs_arr[pf-pf_start].sector = sector;

   if (seg == 1){
        mm[term].unprivileged_seg_page_table[page].pd_r = 1;
   }else if (seg == 2){
        shared_seg_table_page_table[page].pd_r = 1;
   }
   V(&sem_mm);
}

putframe(term)
int term;
{
    int i;
    P(&sem_mm);
    for(i = 0;i<MAXFRAMES;i++){
        if( pfs_arr[i].pid == term && pfs_arr[i].seg != 2){
            pfs_arr[i].pid = -1;
            V(&sem_pf);
        }
    }
    for(i=0;i<32;i++){
        if (mm[term].unprivileged_seg_page_table[i].pd_r == 0 && mm[term].unprivileged_seg_page_table[i].pd_p == 0
            && pfs_arr[i].seg != 2  ){
            int pf = mm[term].unprivileged_seg_page_table[i].pd_frame;
            int track = pf >> 3;
            int sector = pf & 7;
            disk[track][sector] = -1;
        }
    }
    V(&sem_mm);
}

void pagedaemon(){
    while(p_alive > 0){
        P(&sem_page);
        while(sem_pf < HIGH){
            int i;
            int sector = -1;
            int track = -1;
            int page_frame = -1;
            for (i = 0;i<MAXFRAMES;i++){
                P(&sem_mm);
                if (pfs_arr[i].pid != -1){
                    if (pfs_arr[i].seg == 1){
                        int num =  pfs_arr[i].pid;
                        int page = pfs_arr[i].page;
                        if (mm[num].unprivileged_seg_page_table[page].pd_p == 1){
                            // second chance
                            if (mm[num].unprivileged_seg_page_table[page].pd_r == 1){
                                mm[num].unprivileged_seg_page_table[page].pd_r = 0;
                            }else{
                                mm[num].unprivileged_seg_page_table[page].pd_p = 0;
                                if(pfs_arr[i].track != -1 && pfs_arr[i].sector!= -1){
                                    track  = pfs_arr[i].track;
                                    sector = pfs_arr[i].sector;
                                    page_frame = (track << 3) | sector;
                                }else{
                                    page_frame = get_disk_pos();
                                    track = page_frame >> 3;
                                    sector = page_frame & 7;
                                }
                                mm[num].unprivileged_seg_page_table[page].pd_frame = page_frame;
                                page_in_out_helper(track,sector,IOWRITE,pf_start + i);
                                pfs_arr[i].pid = -1;
                                V(&sem_pf);
                            }
                        }

                    }else if (pfs_arr[i].seg == 2){
                        int page = pfs_arr[i].page;
                        if(shared_seg_table_page_table[page].pd_p == 1){
                            if (shared_seg_table_page_table[page].pd_r == 1){
                                shared_seg_table_page_table[page].pd_r = 0;
                            }else{
                                shared_seg_table_page_table[page].pd_p = 0;
                                if(pfs_arr[i].track != -1 && pfs_arr[i].sector!= -1){
                                    track  = pfs_arr[i].track;
                                    sector = pfs_arr[i].sector;
                                    page_frame = (track << 3) | sector;
                                }else{
                                    page_frame = get_disk_pos();
                                    track = page_frame >> 3;
                                    sector = page_frame & 7;
                                }
                                shared_seg_table_page_table[page].pd_frame = page_frame;
                                page_in_out_helper(track,sector,IOWRITE,pf_start + i);
                                pfs_arr[i].pid = -1;
                                V(&sem_pf);
                            }
                        }
                    }
                }
                V(&sem_mm);
            }
        }
    }
    SYS2();
}