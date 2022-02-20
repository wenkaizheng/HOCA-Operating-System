|#physical address used:
|#  1028 is the memory mapped address of the clock register 

	.globl STCK
STCK:
	link.w	%fp,#0
	moveml	#0x0100,-4(%fp)		|# save %a0 on the stack
	move.l	8(%fp),%a0
	move.l	(1028),0(%a0)
	moveml	-4(%fp),#0x0100
LZ1:
	unlk %fp
	rts
