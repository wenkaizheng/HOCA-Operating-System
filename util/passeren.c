#define LOCK -1
#define UNLOCK 1

register int r3 asm("%d3");
register int r4 asm("%d4");

typedef struct vpop {
  int op;
  int *sem;
} vpop; 


passeren(i)
int *i;
{
    vpop semops[2];
  
	semops[0].op = LOCK;
	semops[0].sem = i;
	r3 = 1;
	r4 = (int)semops;
	SYS3();
}
