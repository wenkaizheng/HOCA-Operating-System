
	.data
	HOCA_SP = 60
	HOCA_PC = 64
	HOCA_SR = 68
	HOCA_TMP = 70
	HOCA_CRP = 72
	.text

	.globl STLDSYS13
STLDSYS13:
		link.w	%fp,#0
		moveml	#0x0100,-4(%fp)
		move.l	#0x930,%a0
		moveml	#0x00ff,0(%a0)
		addl	#32,%a0
		move.l	-4(%fp),0(%a0)
		addl	#4,%a0
		moveml	#0x3e00,0(%a0)
		addl	#20,%a0
		move.l	0(%fp),0(%a0)
		addl	#4,%a0
		btst  	#13,4(%fp)
		beq		LaSTLD
		move.l	%fp,0(%a0)
		addl	#10,0(%a0)
		addl	#4,%a0
		move.l	6(%fp),0(%a0)
		addl	#4,%a0
		movew	4(%fp),0(%a0)
		addl	#2,%a0
		movew	#13,0(%a0)
		bra		LbSTLD
	LaSTLD:
		move.l	%usp,%a1
		move.l	%a1,0(%a0)
		addl	#4,%a0
		move.l	6(%fp),0(%a0)
		addl	#4,%a0
		movew	4(%fp),0(%a0)
		addl	#2,%a0
		movew	#13,0(%a0)
	LbSTLD:
		addl	#2,%a0
		move.l	(0x40c),0(%a0)
		andw	#0xefff,%sr
		addl	#4,%a0
		move.l	72(%a0),(0x40c)
		move.l	HOCA_SP(%a0),%a1
		movew	HOCA_SR(%a0),%sr
		move.l	HOCA_PC(%a0),0(%a1)
		movew	HOCA_SR(%a0),%sr
		moveml	0(%a0),#0xffff
		rts
LZ1:
	unlk %fp
	rts
