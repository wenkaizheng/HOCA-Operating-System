	.globl SYS1
SYS1:
	link.w %fp,#0
		pea	1
		trap	#3
LZ1:
	unlk %fp
	rts
	.globl SYS2
SYS2:
	link.w %fp,#0
		pea	2
		trap	#3
LZ2:
	unlk %fp
	rts
	.globl SYS3
SYS3:
	link.w %fp,#0
		pea	3
		trap	#3
LZ3:
	unlk %fp
	rts
	.globl SYS4
SYS4:
	link.w %fp,#0
		pea	4
		trap	#3
LZ4:
	unlk %fp
	rts
	.globl SYS5
SYS5:
	link.w %fp,#0
		pea	5
		trap	#3
LZ5:
	unlk %fp
	rts
	.globl SYS6
SYS6:
	link.w %fp,#0
		pea	6
		trap	#3
LZ6:
	unlk %fp
	rts
	.globl SYS7
SYS7:
	link.w %fp,#0
		pea	7
		trap	#3
LZ7:
	unlk %fp
	rts
	.globl SYS8
SYS8:
	link.w %fp,#0
		pea	8
		trap	#3
LZ8:
	unlk %fp
	rts
	.globl SYS9
SYS9:
	link.w %fp,#0
		trap	#5
LZ9:
	unlk %fp
	rts
	.globl SYS10
SYS10:
	link.w %fp,#0
		trap	#6
LZ10:
	unlk %fp
	rts
	.globl SYS11
SYS11:
	link.w %fp,#0
		trap	#7
LZ11:
	unlk %fp
	rts
	.globl SYS12
SYS12:
	link.w %fp,#0
		trap	#8
LZ12:
	unlk %fp
	rts
	.globl SYS13
SYS13:
	link.w %fp,#0
		trap	#9
LZ13:
	unlk %fp
	rts
	.globl SYS14
SYS14:
	link.w %fp,#0
		trap	#10
LZ14:
	unlk %fp
	rts
	.globl SYS15
SYS15:
	link.w %fp,#0
		trap	#11
LZ15:
	unlk %fp
	rts
	.globl SYS16
SYS16:
	link.w %fp,#0
		trap	#12
LZ16:
	unlk %fp
	rts
	.globl SYS17
SYS17:
	link.w %fp,#0
		trap	#13
LZ17:
	unlk %fp
	rts
