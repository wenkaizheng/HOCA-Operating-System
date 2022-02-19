#include "procq.h"
extern void start_time(proc_t* p);
extern void get_interval(proc_t* p);
extern void pass_up(state_t* old_state, int type);
extern void trapinit();