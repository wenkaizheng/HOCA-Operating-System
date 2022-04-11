
#ifndef SUPPORT_H
#define SUPPORT_H
# include "types.h"
#define  TER 2
typedef struct mm_table{
    char buf[512];
    sd_t unprivileged_seg[32];
    sd_t privileged_seg[32];

    pd_t unprivileged_seg_page_table[32];
    pd_t privileged_seg_page_table[256];

}mm_table;

typedef struct delay_sem{
     int sem_value;
     int d_time;
     long start_time;
}delay_sem;

typedef struct state_area{
    state_t prog_old;
    state_t prog_new;
    state_t mm_old;
    state_t mm_new;
    state_t sys_old;
    state_t sys_new;
}state_area;
#endif
