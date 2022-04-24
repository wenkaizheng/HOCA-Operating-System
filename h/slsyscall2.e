extern void P_Virtual_Semaphore(state_t* old_state, int num);
extern void V_Virtual_Semaphore(state_t* old_state, int num);
extern void Disk_Put(state_t* old_state,int num);
extern void Disk_Get(state_t* old_state,int num);
extern void diskdaemon();