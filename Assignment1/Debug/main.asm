   1              		.syntax unified
   2              		.cpu cortex-m3
   3              		.fpu softvfp
   4              		.eabi_attribute 20, 1
   5              		.eabi_attribute 21, 1
   6              		.eabi_attribute 23, 3
   7              		.eabi_attribute 24, 1
   8              		.eabi_attribute 25, 1
   9              		.eabi_attribute 26, 1
  10              		.eabi_attribute 30, 6
  11              		.eabi_attribute 34, 1
  12              		.eabi_attribute 18, 4
  13              		.thumb
  14              		.file	"main.c"
  15              		.text
  16              	.Ltext0:
  17              		.cfi_sections	.debug_frame
  18              		.section	.text.compute_stats,"ax",%progbits
  19              		.align	2
  20              		.global	compute_stats
  21              		.thumb
  22              		.thumb_func
  24              	compute_stats:
  25              	.LFB29:
  26              		.file 1 "../src/main.c"
   1:../src/main.c **** #include "LPC17xx.h"
   2:../src/main.c **** #include "stdio.h"
   3:../src/main.c **** 
   4:../src/main.c **** // EE2024 Assignment 1 skeleton code
   5:../src/main.c **** // (C) CK Tham, ECE, NUS, 2018
   6:../src/main.c **** 
   7:../src/main.c ****  // Do not modify the following function prototype
   8:../src/main.c **** extern int asm_stats(int* px, int* pvar, int* pmin, int* pmax);
   9:../src/main.c **** 
  10:../src/main.c **** typedef struct data
  11:../src/main.c **** {
  12:../src/main.c ****  	int x[12];
  13:../src/main.c ****  	int mean;
  14:../src/main.c ****  	int variance;
  15:../src/main.c **** 	int min;
  16:../src/main.c ****  	int max;
  17:../src/main.c **** } dataset;
  18:../src/main.c **** 
  19:../src/main.c **** // Do not modify the following function prototype
  20:../src/main.c **** void compute_stats(dataset* dptr)
  21:../src/main.c **** {
  27              		.loc 1 21 0
  28              		.cfi_startproc
  29              		@ args = 0, pretend = 0, frame = 8
  30              		@ frame_needed = 1, uses_anonymous_args = 0
  31 0000 80B5     		push	{r7, lr}
  32              	.LCFI0:
  33              		.cfi_def_cfa_offset 8
  34              		.cfi_offset 14, -4
  35              		.cfi_offset 7, -8
  36 0002 82B0     		sub	sp, sp, #8
  37              	.LCFI1:
  38              		.cfi_def_cfa_offset 16
  39 0004 00AF     		add	r7, sp, #0
  40              	.LCFI2:
  41              		.cfi_def_cfa_register 7
  42 0006 7860     		str	r0, [r7, #4]
  22:../src/main.c **** 	//  Write the code to call asm_stats() function
  23:../src/main.c **** 	//  in an appropriate manner to compute the statistical parameters
  24:../src/main.c **** 	//  ... asm_stats( ... )
  25:../src/main.c **** 
  26:../src/main.c **** 	// addr of mean: &(dptr->mean)
  27:../src/main.c **** 	// addr of variance: &(dptr->varaiance)
  28:../src/main.c **** 	// addr of min: &(dptr->min)
  29:../src/main.c **** 	// addr of max: &(dptr->max)
  30:../src/main.c **** 
  31:../src/main.c **** 	// qns: &(dptr->x[0]) or dptr->x
  32:../src/main.c **** 	dptr->mean = asm_stats(dptr->x, &(dptr->variance), &(dptr->min), &(dptr->max));
  43              		.loc 1 32 0
  44 0008 7868     		ldr	r0, [r7, #4]
  45 000a 7B68     		ldr	r3, [r7, #4]
  46 000c 03F13401 		add	r1, r3, #52
  47 0010 7B68     		ldr	r3, [r7, #4]
  48 0012 03F13802 		add	r2, r3, #56
  49 0016 7B68     		ldr	r3, [r7, #4]
  50 0018 03F13C03 		add	r3, r3, #60
  51 001c FFF7FEFF 		bl	asm_stats
  52 0020 0246     		mov	r2, r0
  53 0022 7B68     		ldr	r3, [r7, #4]
  54 0024 1A63     		str	r2, [r3, #48]
  33:../src/main.c **** }
  55              		.loc 1 33 0
  56 0026 07F10807 		add	r7, r7, #8
  57 002a BD46     		mov	sp, r7
  58 002c 80BD     		pop	{r7, pc}
  59              		.cfi_endproc
  60              	.LFE29:
  62 002e 00BF     		.section	.rodata
  63              		.align	2
  64              	.LC0:
  65 0000 785B2564 		.ascii	"x[%d]: %d\012\000"
  65      5D3A2025 
  65      640A00
  66 000b 00       		.align	2
  67              	.LC1:
  68 000c 6D65616E 		.ascii	"mean: %d\012\000"
  68      3A202564 
  68      0A00
  69 0016 0000     		.align	2
  70              	.LC2:
  71 0018 76617269 		.ascii	"variance: %d\012\000"
  71      616E6365 
  71      3A202564 
  71      0A00
  72 0026 0000     		.align	2
  73              	.LC3:
  74 0028 6D696E3A 		.ascii	"min: %d\012\000"
  74      2025640A 
  74      00
  75 0031 000000   		.align	2
  76              	.LC4:
  77 0034 6D61783A 		.ascii	"max: %d\012\000"
  77      2025640A 
  77      00
  78 003d 000000   		.section	.text.main,"ax",%progbits
  79              		.align	2
  80              		.global	main
  81              		.thumb
  82              		.thumb_func
  84              	main:
  85              	.LFB30:
  34:../src/main.c **** 
  35:../src/main.c **** int main(void)
  36:../src/main.c **** {
  86              		.loc 1 36 0
  87              		.cfi_startproc
  88              		@ args = 0, pretend = 0, frame = 72
  89              		@ frame_needed = 1, uses_anonymous_args = 0
  90 0000 80B5     		push	{r7, lr}
  91              	.LCFI3:
  92              		.cfi_def_cfa_offset 8
  93              		.cfi_offset 14, -4
  94              		.cfi_offset 7, -8
  95 0002 92B0     		sub	sp, sp, #72
  96              	.LCFI4:
  97              		.cfi_def_cfa_offset 80
  98 0004 00AF     		add	r7, sp, #0
  99              	.LCFI5:
 100              		.cfi_def_cfa_register 7
  37:../src/main.c **** 	int i;
  38:../src/main.c **** 	//  Instantiate a dataset object X
  39:../src/main.c **** 	dataset X;
  40:../src/main.c **** 
  41:../src/main.c **** 	//  Initialize the dataset object X
  42:../src/main.c **** 	for (i=0;i<12;i++) {
 101              		.loc 1 42 0
 102 0006 4FF00003 		mov	r3, #0
 103 000a 7B64     		str	r3, [r7, #68]
 104 000c 0EE0     		b	.L3
 105              	.L4:
  43:../src/main.c **** 		X.x[i] = i-6;
 106              		.loc 1 43 0 discriminator 2
 107 000e 7B6C     		ldr	r3, [r7, #68]
 108 0010 A3F10602 		sub	r2, r3, #6
 109 0014 7B6C     		ldr	r3, [r7, #68]
 110 0016 4FEA8303 		lsl	r3, r3, #2
 111 001a 07F14801 		add	r1, r7, #72
 112 001e CB18     		adds	r3, r1, r3
 113 0020 43F8482C 		str	r2, [r3, #-72]
  42:../src/main.c **** 	for (i=0;i<12;i++) {
 114              		.loc 1 42 0 discriminator 2
 115 0024 7B6C     		ldr	r3, [r7, #68]
 116 0026 03F10103 		add	r3, r3, #1
 117 002a 7B64     		str	r3, [r7, #68]
 118              	.L3:
  42:../src/main.c **** 	for (i=0;i<12;i++) {
 119              		.loc 1 42 0 is_stmt 0 discriminator 1
 120 002c 7B6C     		ldr	r3, [r7, #68]
 121 002e 0B2B     		cmp	r3, #11
 122 0030 EDDD     		ble	.L4
  44:../src/main.c **** 	}
  45:../src/main.c **** 	X.mean = X.variance = X.min = X.max = 0;
 123              		.loc 1 45 0 is_stmt 1
 124 0032 4FF00003 		mov	r3, #0
 125 0036 FB63     		str	r3, [r7, #60]
 126 0038 FB6B     		ldr	r3, [r7, #60]
 127 003a BB63     		str	r3, [r7, #56]
 128 003c BB6B     		ldr	r3, [r7, #56]
 129 003e 7B63     		str	r3, [r7, #52]
 130 0040 7B6B     		ldr	r3, [r7, #52]
 131 0042 3B63     		str	r3, [r7, #48]
  46:../src/main.c **** 
  47:../src/main.c **** 	// define a pointer of dataset
  48:../src/main.c **** 	dataset* ptr_X;
  49:../src/main.c **** 	ptr_X = &X;
 132              		.loc 1 49 0
 133 0044 3B46     		mov	r3, r7
 134 0046 3B64     		str	r3, [r7, #64]
  50:../src/main.c **** 
  51:../src/main.c **** 	//  Call compute_stats() function in an appropriate manner to process the dataset object X
  52:../src/main.c **** 	compute_stats(ptr_X); //doing cal of
 135              		.loc 1 52 0
 136 0048 386C     		ldr	r0, [r7, #64]
 137 004a FFF7FEFF 		bl	compute_stats
  53:../src/main.c **** 
  54:../src/main.c **** 	//  Print out the contents of the dataset object X
  55:../src/main.c **** 	for (i=0;i<12;i++)
 138              		.loc 1 55 0
 139 004e 4FF00003 		mov	r3, #0
 140 0052 7B64     		str	r3, [r7, #68]
 141 0054 13E0     		b	.L5
 142              	.L6:
  56:../src/main.c **** 		printf("x[%d]: %d\n", i, X.x[i]);
 143              		.loc 1 56 0 discriminator 2
 144 0056 40F20003 		movw	r3, #:lower16:.LC0
 145 005a C0F20003 		movt	r3, #:upper16:.LC0
 146 005e 7A6C     		ldr	r2, [r7, #68]
 147 0060 4FEA8202 		lsl	r2, r2, #2
 148 0064 07F14801 		add	r1, r7, #72
 149 0068 8A18     		adds	r2, r1, r2
 150 006a 52F8482C 		ldr	r2, [r2, #-72]
 151 006e 1846     		mov	r0, r3
 152 0070 796C     		ldr	r1, [r7, #68]
 153 0072 FFF7FEFF 		bl	printf
  55:../src/main.c **** 	for (i=0;i<12;i++)
 154              		.loc 1 55 0 discriminator 2
 155 0076 7B6C     		ldr	r3, [r7, #68]
 156 0078 03F10103 		add	r3, r3, #1
 157 007c 7B64     		str	r3, [r7, #68]
 158              	.L5:
  55:../src/main.c **** 	for (i=0;i<12;i++)
 159              		.loc 1 55 0 is_stmt 0 discriminator 1
 160 007e 7B6C     		ldr	r3, [r7, #68]
 161 0080 0B2B     		cmp	r3, #11
 162 0082 E8DD     		ble	.L6
  57:../src/main.c **** 	printf("mean: %d\n", X.mean);
 163              		.loc 1 57 0 is_stmt 1
 164 0084 40F20003 		movw	r3, #:lower16:.LC1
 165 0088 C0F20003 		movt	r3, #:upper16:.LC1
 166 008c 3A6B     		ldr	r2, [r7, #48]
 167 008e 1846     		mov	r0, r3
 168 0090 1146     		mov	r1, r2
 169 0092 FFF7FEFF 		bl	printf
  58:../src/main.c **** 	printf("variance: %d\n", X.variance);
 170              		.loc 1 58 0
 171 0096 40F20003 		movw	r3, #:lower16:.LC2
 172 009a C0F20003 		movt	r3, #:upper16:.LC2
 173 009e 7A6B     		ldr	r2, [r7, #52]
 174 00a0 1846     		mov	r0, r3
 175 00a2 1146     		mov	r1, r2
 176 00a4 FFF7FEFF 		bl	printf
  59:../src/main.c **** 	printf("min: %d\n", X.min);
 177              		.loc 1 59 0
 178 00a8 40F20003 		movw	r3, #:lower16:.LC3
 179 00ac C0F20003 		movt	r3, #:upper16:.LC3
 180 00b0 BA6B     		ldr	r2, [r7, #56]
 181 00b2 1846     		mov	r0, r3
 182 00b4 1146     		mov	r1, r2
 183 00b6 FFF7FEFF 		bl	printf
  60:../src/main.c **** 	printf("max: %d\n", X.max);
 184              		.loc 1 60 0
 185 00ba 40F20003 		movw	r3, #:lower16:.LC4
 186 00be C0F20003 		movt	r3, #:upper16:.LC4
 187 00c2 FA6B     		ldr	r2, [r7, #60]
 188 00c4 1846     		mov	r0, r3
 189 00c6 1146     		mov	r1, r2
 190 00c8 FFF7FEFF 		bl	printf
 191              	.L7:
  61:../src/main.c **** 
  62:../src/main.c **** 	// Enter an infinite loop, just incrementing a counter.
  63:../src/main.c **** 	// Do not modify the code below. It enables values or variables and registers to be inspected befo
  64:../src/main.c **** 	volatile static int loop = 0;
  65:../src/main.c **** 	while (1) {
  66:../src/main.c **** 		loop++;
 192              		.loc 1 66 0 discriminator 1
 193 00cc 40F20003 		movw	r3, #:lower16:loop.4860
 194 00d0 C0F20003 		movt	r3, #:upper16:loop.4860
 195 00d4 1B68     		ldr	r3, [r3, #0]
 196 00d6 03F10102 		add	r2, r3, #1
 197 00da 40F20003 		movw	r3, #:lower16:loop.4860
 198 00de C0F20003 		movt	r3, #:upper16:loop.4860
 199 00e2 1A60     		str	r2, [r3, #0]
  67:../src/main.c **** 	}
 200              		.loc 1 67 0 discriminator 1
 201 00e4 F2E7     		b	.L7
 202              		.cfi_endproc
 203              	.LFE30:
 205 00e6 00BF     		.bss
 206              		.align	2
 207              	loop.4860:
 208 0000 00000000 		.space	4
 209              		.text
 210              	.Letext0:
 211              		.file 2 "C:\\Users\\bokwoon\\Documents\\LPCXpresso_6.1.4_194\\workspace\\Lib_CMSISv1p30_LPC17xx\\i
DEFINED SYMBOLS
                            *ABS*:00000000 main.c
C:\Users\bokwoon\AppData\Local\Temp\ccLRF9sQ.s:19     .text.compute_stats:00000000 $t
C:\Users\bokwoon\AppData\Local\Temp\ccLRF9sQ.s:24     .text.compute_stats:00000000 compute_stats
C:\Users\bokwoon\AppData\Local\Temp\ccLRF9sQ.s:63     .rodata:00000000 $d
C:\Users\bokwoon\AppData\Local\Temp\ccLRF9sQ.s:64     .rodata:00000000 .LC0
C:\Users\bokwoon\AppData\Local\Temp\ccLRF9sQ.s:67     .rodata:0000000c .LC1
C:\Users\bokwoon\AppData\Local\Temp\ccLRF9sQ.s:70     .rodata:00000018 .LC2
C:\Users\bokwoon\AppData\Local\Temp\ccLRF9sQ.s:73     .rodata:00000028 .LC3
C:\Users\bokwoon\AppData\Local\Temp\ccLRF9sQ.s:76     .rodata:00000034 .LC4
C:\Users\bokwoon\AppData\Local\Temp\ccLRF9sQ.s:79     .text.main:00000000 $t
C:\Users\bokwoon\AppData\Local\Temp\ccLRF9sQ.s:84     .text.main:00000000 main
C:\Users\bokwoon\AppData\Local\Temp\ccLRF9sQ.s:207    .bss:00000000 loop.4860
C:\Users\bokwoon\AppData\Local\Temp\ccLRF9sQ.s:206    .bss:00000000 $d
                     .debug_frame:00000010 $d

UNDEFINED SYMBOLS
asm_stats
printf
