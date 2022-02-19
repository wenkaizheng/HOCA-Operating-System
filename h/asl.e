#include "asl.h"
#include "procq.h"
extern int insertBlocked(int *semAdd, proc_t *p);
extern proc_t *removeBlocked(int *semAdd);
extern proc_t *outBlocked(proc_t *p);
extern proc_t *headBlocked(int *semAdd);
extern int initSemd();
extern int headASL();
