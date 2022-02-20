|# physical address used:
|# 0x408 = 1032  register of IT is memory mapped to 0x408

	.globl LDIT
LDIT:
	link.w %fp,#0
	moveml	#0x0100,-4(%fp)   |# save %a0 on the stack
	move.l	8(%fp),%a0
	move.l	0(%a0),1032
	moveml	-4(%fp),#0x0100
LZ1:
	unlk %fp
	rts
