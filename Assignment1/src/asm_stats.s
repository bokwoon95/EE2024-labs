 	.syntax unified
 	.cpu cortex-m3
 	.thumb
 	.align 2
 	.global	asm_stats
 	.thumb_func

asm_stats:
PUSH {R4-R12}

	@ this block calculates the size of the array and stores it in R4
	SUB R4, R1, R0 @address of variance minus address of array
	MOV R5, #4
	UDIV R4, R4, R5 @divide the difference by 4 bits to get (size of array + 1)
	SUB R4, R4, #1 @subtract 1 to get size of array
	MOV R5, R4

	MOV R6, R5
	MOV R7, #0 @initialize r6 as 0
loop0: @loop0 sums up array (to calculate mean)
	LDR R8, [R0], #4 @store each array element in r8
	ADD R7, R8 @add each array element r8 to running sum r7
	SUBS R6, #1 @subtract 1 from loop counter
	BGT loop0
	SUB R0, R0, R5, LSL #2 @restore position of r0 back to first element of array by subtracting arraysize(r5)*4

	SDIV R12, R7, R5 @divide array sum r7 by array size r5 to get mean r12

	MOV R6, R5 @copy array size r5 back to loop counter r6
	MOV R11, #0 @initialize variance r11 as 0

loop1: @loop1
	LDR R8, [R0], #4 @increment through the array
	MOV R7, R8 @ store array element in R8
	SUB R7, R12
	MOV R7, R8 @Both R7 & R8 are -6
	MUL R7, R8 @Why is R7*R8 = -28?
	ADD R11, R7
	SUBS R6, R6, #1
	BGT loop1
	SUB R0, R0, R5, LSL #2 @restore position of r0 back to first element of array by subtracting arraysize(r5)*4

	MOV R6, R5
	SUB R6, #1
	SDIV R11, R6

	// Finding Min and Max
	// Min: R10 Max: R9
	MOV R6, R5 @copy array size r5 back to loop counter r6
	LDR R10, [R0]
	LDR R9, [R0]
loop2:
	LDR R8, [R0], #4 @increment through the array

	CMP R8, R10
	IT LT
	MOVLT R10, R8

	CMP R8, R9
	IT GT
	MOVGT R9, R8

	SUBS R5, #1

	BGT loop2

	MOV R0, R12
	STR R11, [R1]
	STR R10, [R2]
	STR R9, [R3]

	POP {R4-R12}

	BX	LR

