	.globl STST
STST:
	link.w	%fp,#0
	moveml	#0x0100,-4(%fp)		|# save %a0 on stack
	movew	%sr,%fp@(-6)		|# save condition codes
	move.l	8(%fp),%a0
	moveml	#0x00ff,0(%a0)		|# data registers
	addl	#32,%a0
	move.l	-4(%fp),0(%a0)		|# %a0
	addl	#4,%a0
	moveml	#0x3e00,0(%a0)		|# %a1 - a5
	addl	#20,%a0
	move.l	0(%fp),0(%a0)		|# %fp = fp
	addl	#4,%a0
	move.l	%fp,0(%a0)		|# a7
	addl	#4,%a0
	move.l	4(%fp),0(%a0)		|# pc
	addl	#4,%a0
	movew	%fp@(-6),0(%a0)		|# %sr
	addl	#4,%a0
	|# Memory Managment Instruction 	# crp
	move.l	(0x40c),0(%a0)
	moveml	-4(%fp),#0x0100		|# restore %a0
	unlk %fp
	rts
