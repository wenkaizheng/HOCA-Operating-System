|# physical address used:
|# 0x40c = 1036 for CRP  

	.data
	HOCA_D0 =  0
	HOCA_D1 =  4
	HOCA_D2 =  8
	HOCA_D3 = 12
	HOCA_D4 = 16
	HOCA_D5 = 20
	HOCA_D6 = 24
	HOCA_D7 = 28
	HOCA_A0 = 32
	HOCA_A1 = 36
	HOCA_A2 = 40
	HOCA_A3 = 44
	HOCA_A4 = 48
	HOCA_A5 = 52
	HOCA_A6 = 56
	HOCA_SP = 60
	HOCA_PC = 64
	HOCA_SR = 68
	HOCA_TMP = 70
	HOCA_CRP = 72
	.text

	.globl LDST
LDST:
	link.w %fp,#0
	and.w	#0xefff,%sr		|# disable MM
	or.w	#0x0700,%sr		|# disable MM
	move.l	8(%fp),%a0		|# store argument in %a0
	move.w	HOCA_SR(%a0),%d0	|# store SR in d0
	btst	#13,%d0			|# will the process be in user mode ?
	beq		LaLDST
|#
|# process will be in supervisor mode
	move.l	HOCA_SP(%a0),%sp	|# store the new SP 
	sub.l	#6,%sp			|# alloc space for ps, pc
	move.l	HOCA_PC(%a0),2(%sp)	|# store PC in the new SP
	move.w	HOCA_SR(%a0),0(%sp)	|# store PS in the new SP
	move.l  72(%a0),(0x40c)     	|# load MM register 
	movem.l	0(%a0),#0x7fff		|# load Dn and An
	rte
|# process will be in user mode
	LaLDST:
	move.l	HOCA_SP(%a0),%a1	|# store new SP in %a1
	sub.l	#6,%a7			|# alloc space in SSP for PS and PC
	move.l	HOCA_PC(%a0),2(%a7)	|# store PC in SSP
	move.w	%d0,0(%a7)		|# store SR in SSP
	move.l  72(%a0),(0x40c)
	move.l	%a1,%usp		|# load the USP
	movem.l	0(%a0),#0x7fff		|# load Dn and An 
	rte
LZ1:
	unlk %fp
	rts

