|# the purpose of this routine is to save the processor state in
|# the Old Area and to load a new processor state from the New area
|#
|# note that the device number is stored in HOCA_TMP
|#
|# physical address used:
|#  0x800     for the old disk int area
|#


	.data
	HOCA_SP = 60
	HOCA_PC = 64
	HOCA_SR = 68
	HOCA_TMP = 70
	HOCA_CRP = 72
	.text

	.globl STLDADDRESS
STLDADDRESS:
	link.w	%fp,#0
|#
|# save current processor state in the Old Area
|#
	movem.l	#0x0100,-4(%fp)		|# save %a0 on the stack
	move.l	#0x800,%a0		|# address of old area to %a0 
	moveml	#0x00ff,0(%a0)		|# D0 - D7
	addl	#32,%a0
	move.l	-4(%fp),0(%a0)		|# %a0
	addl	#4,%a0
	moveml	#0x3e00,0(%a0)		|# %a1 - a5
	addl	#20,%a0
	move.l	0(%fp),0(%a0)		|# %fp = fp
	addl	#4,%a0
	btst  	#13,4(%fp)		|# supervisor or user mode ?
	beq		LaSTLD
|#  Supervisor Mode
	move.l	%fp,0(%a0)		|# SSP, a7
	addl	#10,0(%a0)		|# ignore %fp, SR and PC 
	bra		LbSTLD
|#  User Mode
	LaSTLD:
	move.l	%usp,%a1
	move.l	%a1,0(%a0)
	LbSTLD:
	addl	#4,%a0
	move.l	6(%fp),0(%a0)		|# PC
	addl	#4,%a0
	movew	4(%fp),0(%a0)		|# SR
	addl	#2,%a0
	movew	#3,0(%a0)		|# EVT # for address error
	addl	#2,%a0
	move.l	(0x40c),0(%a0)
	andw	#0xefff,%sr
	addl	#4,%a0			|# advance to the new state
|# load the processor state from the New Area
	move.l	HOCA_CRP(%a0),(0x40c)
	move.l	HOCA_SP(%a0),%a1	|# store new SP in %a1
	movew	HOCA_SR(%a0),%sr	|# enable mm, intrs should be disabled
	move.l	HOCA_PC(%a0),0(%a1)	|# copy PC to new SP
	movew	HOCA_SR(%a0),%cc	|# reload condition codes
	moveml	0(%a0),#0xffff		|# load Dn and An
	rts				|# load the PC
LZ1:
	unlk %fp
	rts
