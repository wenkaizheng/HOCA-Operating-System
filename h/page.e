#include  "support.h"
extern int p_alive;
extern int sem_mm;
extern int sem_disk;
extern int protected_sem;
extern int Scronframe;
extern int Scronstack;
extern int Tsysframe[5];
extern int Tmmframe[5];
extern int Tsysstack[5];
extern int Tmmstack[5];
extern diks_scheduler ds_arr[MAXPROC];
extern virtual_sem vs_arr[MAXPROC];
extern void P(int* sem_value);
extern void V(int* sem_value);