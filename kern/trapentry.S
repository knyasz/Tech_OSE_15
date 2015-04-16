/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>



###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */

 /*
From http://wiki.osdev.org/Exceptions
Exceptions that return error code (use TRAPHANDLER):

Name							Vector nr.		Type	Mnemonic	Error code?
Divide-by-zero Error			0 (0x0)			Fault		#DE			No
Debug							1 (0x1)			Fault/Trap	#DB			No
Non-maskable Interrupt			2 (0x2)			Interrupt	-			No
Breakpoint						3 (0x3)			Trap		#BP			No
Overflow						4 (0x4)			Trap		#OF			No
Bound Range Exceeded			5 (0x5)			Fault		#BR			No
Invalid Opcode					6 (0x6)			Fault		#UD			No
Device Not Available			7 (0x7)			Fault		#NM			No
Double Fault					8 (0x8)			Abort		#DF			Yes
Coprocessor Segment Overrun		9 (0x9)			Fault		-			No
Invalid TSS						10 (0xA)		Fault		#TS			Yes
Segment Not Present				11 (0xB)		Fault		#NP			Yes
Stack-Segment Fault				12 (0xC)		Fault		#SS			Yes
General Protection Fault		13 (0xD)		Fault		#GP			Yes
Page Fault						14 (0xE)		Fault		#PF			Yes
Reserved						15 (0xF)		-			-			No
x87 Floating-Point Exception	16 (0x10)		Fault		#MF			No
Alignment Check					17 (0x11)		Fault		#AC			Yes
Machine Check					18 (0x12)		Abort		#MC			No
SIMD Floating-Point Exception	19 (0x13)		Fault		#XM/#XF		No
Virtualization Exception		20 (0x14)		Fault		#VE			No
Reserved						21-29 (0x15-0x1D)	-		-			No
Security Exception				30 (0x1E)		-			#SX			Yes
Reserved						31 (0x1F)		-			-			No
Triple Fault					-				-			-			No
*/
TRAPHANDLER_NOEC(interrupt_vector0, T_DIVIDE)
TRAPHANDLER_NOEC(interrupt_vector1, T_DEBUG)
TRAPHANDLER_NOEC(interrupt_vector2, T_NMI)
TRAPHANDLER_NOEC(interrupt_vector3, T_BRKPT)
TRAPHANDLER_NOEC(interrupt_vector4, T_OFLOW)
TRAPHANDLER_NOEC(interrupt_vector5, T_BOUND)
TRAPHANDLER_NOEC(interrupt_vector6, T_ILLOP)
TRAPHANDLER_NOEC(interrupt_vector7, T_DEVICE)
TRAPHANDLER(interrupt_vector8, T_DBLFLT)
TRAPHANDLER_NOEC(interrupt_vector9, T_COPROC)	/* Reserved */
TRAPHANDLER(interrupt_vector10, T_TSS)
TRAPHANDLER(interrupt_vector11, T_SEGNP)
TRAPHANDLER(interrupt_vector12, T_STACK)
TRAPHANDLER(interrupt_vector13, T_GPFLT)
TRAPHANDLER(interrupt_vector14, T_PGFLT)
TRAPHANDLER_NOEC(interrupt_vector15, T_RES)		/* Reserved */
TRAPHANDLER_NOEC(interrupt_vector16, T_FPERR)
TRAPHANDLER(interrupt_vector17, T_ALIGN)
TRAPHANDLER_NOEC(interrupt_vector18, T_MCHK)
TRAPHANDLER_NOEC(interrupt_vector19, T_SIMDERR)
TRAPHANDLER_NOEC(interrupt_vector20, 20)
TRAPHANDLER_NOEC(interrupt_vector21, 21)		/* Reserved */
TRAPHANDLER_NOEC(interrupt_vector22, 22)		/* Reserved */
TRAPHANDLER_NOEC(interrupt_vector23, 23)		/* Reserved */
TRAPHANDLER_NOEC(interrupt_vector24, 24)		/* Reserved */
TRAPHANDLER_NOEC(interrupt_vector25, 25)		/* Reserved */
TRAPHANDLER_NOEC(interrupt_vector26, 26)		/* Reserved */
TRAPHANDLER_NOEC(interrupt_vector27, 27)		/* Reserved */
TRAPHANDLER_NOEC(interrupt_vector28, 28)		/* Reserved */
TRAPHANDLER_NOEC(interrupt_vector29, 29)		/* Reserved */
TRAPHANDLER(interrupt_vector30, 30)
TRAPHANDLER_NOEC(interrupt_vector31, 31)		/* Reserved */
TRAPHANDLER_NOEC(interrupt_vector32, 32)
TRAPHANDLER_NOEC(interrupt_vector33, 33)
TRAPHANDLER_NOEC(interrupt_vector34, 34)
TRAPHANDLER_NOEC(interrupt_vector35, 35)
TRAPHANDLER_NOEC(interrupt_vector36, 36)
TRAPHANDLER_NOEC(interrupt_vector37, 37)
TRAPHANDLER_NOEC(interrupt_vector38, 38)
TRAPHANDLER_NOEC(interrupt_vector39, 39)
TRAPHANDLER_NOEC(interrupt_vector40, 40)
TRAPHANDLER_NOEC(interrupt_vector41, 41)
TRAPHANDLER_NOEC(interrupt_vector42, 42)
TRAPHANDLER_NOEC(interrupt_vector43, 43)
TRAPHANDLER_NOEC(interrupt_vector44, 44)
TRAPHANDLER_NOEC(interrupt_vector45, 45)
TRAPHANDLER_NOEC(interrupt_vector46, 46)
TRAPHANDLER_NOEC(interrupt_vector47, 47)
TRAPHANDLER_NOEC(interrupt_vector48, T_SYSCALL)
TRAPHANDLER_NOEC(interrupt_vector49, 49)
TRAPHANDLER_NOEC(interrupt_vector50, 50)
TRAPHANDLER_NOEC(interrupt_vector51, 51)
TRAPHANDLER_NOEC(interrupt_vector52, 52)
TRAPHANDLER_NOEC(interrupt_vector53, 53)
TRAPHANDLER_NOEC(interrupt_vector54, 54)
TRAPHANDLER_NOEC(interrupt_vector55, 55)
TRAPHANDLER_NOEC(interrupt_vector56, 56)
TRAPHANDLER_NOEC(interrupt_vector57, 57)
TRAPHANDLER_NOEC(interrupt_vector58, 58)
TRAPHANDLER_NOEC(interrupt_vector59, 59)
TRAPHANDLER_NOEC(interrupt_vector60, 60)
TRAPHANDLER_NOEC(interrupt_vector61, 61)
TRAPHANDLER_NOEC(interrupt_vector62, 62)
TRAPHANDLER_NOEC(interrupt_vector63, 63)
TRAPHANDLER_NOEC(interrupt_vector64, 64)
TRAPHANDLER_NOEC(interrupt_vector65, 65)
TRAPHANDLER_NOEC(interrupt_vector66, 66)
TRAPHANDLER_NOEC(interrupt_vector67, 67)
TRAPHANDLER_NOEC(interrupt_vector68, 68)
TRAPHANDLER_NOEC(interrupt_vector69, 69)
TRAPHANDLER_NOEC(interrupt_vector70, 70)
TRAPHANDLER_NOEC(interrupt_vector71, 71)
TRAPHANDLER_NOEC(interrupt_vector72, 72)
TRAPHANDLER_NOEC(interrupt_vector73, 73)
TRAPHANDLER_NOEC(interrupt_vector74, 74)
TRAPHANDLER_NOEC(interrupt_vector75, 75)
TRAPHANDLER_NOEC(interrupt_vector76, 76)
TRAPHANDLER_NOEC(interrupt_vector77, 77)
TRAPHANDLER_NOEC(interrupt_vector78, 78)
TRAPHANDLER_NOEC(interrupt_vector79, 79)
TRAPHANDLER_NOEC(interrupt_vector80, 80)
TRAPHANDLER_NOEC(interrupt_vector81, 81)
TRAPHANDLER_NOEC(interrupt_vector82, 82)
TRAPHANDLER_NOEC(interrupt_vector83, 83)
TRAPHANDLER_NOEC(interrupt_vector84, 84)
TRAPHANDLER_NOEC(interrupt_vector85, 85)
TRAPHANDLER_NOEC(interrupt_vector86, 86)
TRAPHANDLER_NOEC(interrupt_vector87, 87)
TRAPHANDLER_NOEC(interrupt_vector88, 88)
TRAPHANDLER_NOEC(interrupt_vector89, 89)
TRAPHANDLER_NOEC(interrupt_vector90, 90)
TRAPHANDLER_NOEC(interrupt_vector91, 91)
TRAPHANDLER_NOEC(interrupt_vector92, 92)
TRAPHANDLER_NOEC(interrupt_vector93, 93)
TRAPHANDLER_NOEC(interrupt_vector94, 94)
TRAPHANDLER_NOEC(interrupt_vector95, 95)
TRAPHANDLER_NOEC(interrupt_vector96, 96)
TRAPHANDLER_NOEC(interrupt_vector97, 97)
TRAPHANDLER_NOEC(interrupt_vector98, 98)
TRAPHANDLER_NOEC(interrupt_vector99, 99)
TRAPHANDLER_NOEC(interrupt_vector100, 100)
TRAPHANDLER_NOEC(interrupt_vector101, 101)
TRAPHANDLER_NOEC(interrupt_vector102, 102)
TRAPHANDLER_NOEC(interrupt_vector103, 103)
TRAPHANDLER_NOEC(interrupt_vector104, 104)
TRAPHANDLER_NOEC(interrupt_vector105, 105)
TRAPHANDLER_NOEC(interrupt_vector106, 106)
TRAPHANDLER_NOEC(interrupt_vector107, 107)
TRAPHANDLER_NOEC(interrupt_vector108, 108)
TRAPHANDLER_NOEC(interrupt_vector109, 109)
TRAPHANDLER_NOEC(interrupt_vector110, 110)
TRAPHANDLER_NOEC(interrupt_vector111, 111)
TRAPHANDLER_NOEC(interrupt_vector112, 112)
TRAPHANDLER_NOEC(interrupt_vector113, 113)
TRAPHANDLER_NOEC(interrupt_vector114, 114)
TRAPHANDLER_NOEC(interrupt_vector115, 115)
TRAPHANDLER_NOEC(interrupt_vector116, 116)
TRAPHANDLER_NOEC(interrupt_vector117, 117)
TRAPHANDLER_NOEC(interrupt_vector118, 118)
TRAPHANDLER_NOEC(interrupt_vector119, 119)
TRAPHANDLER_NOEC(interrupt_vector120, 120)
TRAPHANDLER_NOEC(interrupt_vector121, 121)
TRAPHANDLER_NOEC(interrupt_vector122, 122)
TRAPHANDLER_NOEC(interrupt_vector123, 123)
TRAPHANDLER_NOEC(interrupt_vector124, 124)
TRAPHANDLER_NOEC(interrupt_vector125, 125)
TRAPHANDLER_NOEC(interrupt_vector126, 126)
TRAPHANDLER_NOEC(interrupt_vector127, 127)
TRAPHANDLER_NOEC(interrupt_vector128, 128)
TRAPHANDLER_NOEC(interrupt_vector129, 129)
TRAPHANDLER_NOEC(interrupt_vector130, 130)
TRAPHANDLER_NOEC(interrupt_vector131, 131)
TRAPHANDLER_NOEC(interrupt_vector132, 132)
TRAPHANDLER_NOEC(interrupt_vector133, 133)
TRAPHANDLER_NOEC(interrupt_vector134, 134)
TRAPHANDLER_NOEC(interrupt_vector135, 135)
TRAPHANDLER_NOEC(interrupt_vector136, 136)
TRAPHANDLER_NOEC(interrupt_vector137, 137)
TRAPHANDLER_NOEC(interrupt_vector138, 138)
TRAPHANDLER_NOEC(interrupt_vector139, 139)
TRAPHANDLER_NOEC(interrupt_vector140, 140)
TRAPHANDLER_NOEC(interrupt_vector141, 141)
TRAPHANDLER_NOEC(interrupt_vector142, 142)
TRAPHANDLER_NOEC(interrupt_vector143, 143)
TRAPHANDLER_NOEC(interrupt_vector144, 144)
TRAPHANDLER_NOEC(interrupt_vector145, 145)
TRAPHANDLER_NOEC(interrupt_vector146, 146)
TRAPHANDLER_NOEC(interrupt_vector147, 147)
TRAPHANDLER_NOEC(interrupt_vector148, 148)
TRAPHANDLER_NOEC(interrupt_vector149, 149)
TRAPHANDLER_NOEC(interrupt_vector150, 150)
TRAPHANDLER_NOEC(interrupt_vector151, 151)
TRAPHANDLER_NOEC(interrupt_vector152, 152)
TRAPHANDLER_NOEC(interrupt_vector153, 153)
TRAPHANDLER_NOEC(interrupt_vector154, 154)
TRAPHANDLER_NOEC(interrupt_vector155, 155)
TRAPHANDLER_NOEC(interrupt_vector156, 156)
TRAPHANDLER_NOEC(interrupt_vector157, 157)
TRAPHANDLER_NOEC(interrupt_vector158, 158)
TRAPHANDLER_NOEC(interrupt_vector159, 159)
TRAPHANDLER_NOEC(interrupt_vector160, 160)
TRAPHANDLER_NOEC(interrupt_vector161, 161)
TRAPHANDLER_NOEC(interrupt_vector162, 162)
TRAPHANDLER_NOEC(interrupt_vector163, 163)
TRAPHANDLER_NOEC(interrupt_vector164, 164)
TRAPHANDLER_NOEC(interrupt_vector165, 165)
TRAPHANDLER_NOEC(interrupt_vector166, 166)
TRAPHANDLER_NOEC(interrupt_vector167, 167)
TRAPHANDLER_NOEC(interrupt_vector168, 168)
TRAPHANDLER_NOEC(interrupt_vector169, 169)
TRAPHANDLER_NOEC(interrupt_vector170, 170)
TRAPHANDLER_NOEC(interrupt_vector171, 171)
TRAPHANDLER_NOEC(interrupt_vector172, 172)
TRAPHANDLER_NOEC(interrupt_vector173, 173)
TRAPHANDLER_NOEC(interrupt_vector174, 174)
TRAPHANDLER_NOEC(interrupt_vector175, 175)
TRAPHANDLER_NOEC(interrupt_vector176, 176)
TRAPHANDLER_NOEC(interrupt_vector177, 177)
TRAPHANDLER_NOEC(interrupt_vector178, 178)
TRAPHANDLER_NOEC(interrupt_vector179, 179)
TRAPHANDLER_NOEC(interrupt_vector180, 180)
TRAPHANDLER_NOEC(interrupt_vector181, 181)
TRAPHANDLER_NOEC(interrupt_vector182, 182)
TRAPHANDLER_NOEC(interrupt_vector183, 183)
TRAPHANDLER_NOEC(interrupt_vector184, 184)
TRAPHANDLER_NOEC(interrupt_vector185, 185)
TRAPHANDLER_NOEC(interrupt_vector186, 186)
TRAPHANDLER_NOEC(interrupt_vector187, 187)
TRAPHANDLER_NOEC(interrupt_vector188, 188)
TRAPHANDLER_NOEC(interrupt_vector189, 189)
TRAPHANDLER_NOEC(interrupt_vector190, 190)
TRAPHANDLER_NOEC(interrupt_vector191, 191)
TRAPHANDLER_NOEC(interrupt_vector192, 192)
TRAPHANDLER_NOEC(interrupt_vector193, 193)
TRAPHANDLER_NOEC(interrupt_vector194, 194)
TRAPHANDLER_NOEC(interrupt_vector195, 195)
TRAPHANDLER_NOEC(interrupt_vector196, 196)
TRAPHANDLER_NOEC(interrupt_vector197, 197)
TRAPHANDLER_NOEC(interrupt_vector198, 198)
TRAPHANDLER_NOEC(interrupt_vector199, 199)
TRAPHANDLER_NOEC(interrupt_vector200, 200)
TRAPHANDLER_NOEC(interrupt_vector201, 201)
TRAPHANDLER_NOEC(interrupt_vector202, 202)
TRAPHANDLER_NOEC(interrupt_vector203, 203)
TRAPHANDLER_NOEC(interrupt_vector204, 204)
TRAPHANDLER_NOEC(interrupt_vector205, 205)
TRAPHANDLER_NOEC(interrupt_vector206, 206)
TRAPHANDLER_NOEC(interrupt_vector207, 207)
TRAPHANDLER_NOEC(interrupt_vector208, 208)
TRAPHANDLER_NOEC(interrupt_vector209, 209)
TRAPHANDLER_NOEC(interrupt_vector210, 210)
TRAPHANDLER_NOEC(interrupt_vector211, 211)
TRAPHANDLER_NOEC(interrupt_vector212, 212)
TRAPHANDLER_NOEC(interrupt_vector213, 213)
TRAPHANDLER_NOEC(interrupt_vector214, 214)
TRAPHANDLER_NOEC(interrupt_vector215, 215)
TRAPHANDLER_NOEC(interrupt_vector216, 216)
TRAPHANDLER_NOEC(interrupt_vector217, 217)
TRAPHANDLER_NOEC(interrupt_vector218, 218)
TRAPHANDLER_NOEC(interrupt_vector219, 219)
TRAPHANDLER_NOEC(interrupt_vector220, 220)
TRAPHANDLER_NOEC(interrupt_vector221, 221)
TRAPHANDLER_NOEC(interrupt_vector222, 222)
TRAPHANDLER_NOEC(interrupt_vector223, 223)
TRAPHANDLER_NOEC(interrupt_vector224, 224)
TRAPHANDLER_NOEC(interrupt_vector225, 225)
TRAPHANDLER_NOEC(interrupt_vector226, 226)
TRAPHANDLER_NOEC(interrupt_vector227, 227)
TRAPHANDLER_NOEC(interrupt_vector228, 228)
TRAPHANDLER_NOEC(interrupt_vector229, 229)
TRAPHANDLER_NOEC(interrupt_vector230, 230)
TRAPHANDLER_NOEC(interrupt_vector231, 231)
TRAPHANDLER_NOEC(interrupt_vector232, 232)
TRAPHANDLER_NOEC(interrupt_vector233, 233)
TRAPHANDLER_NOEC(interrupt_vector234, 234)
TRAPHANDLER_NOEC(interrupt_vector235, 235)
TRAPHANDLER_NOEC(interrupt_vector236, 236)
TRAPHANDLER_NOEC(interrupt_vector237, 237)
TRAPHANDLER_NOEC(interrupt_vector238, 238)
TRAPHANDLER_NOEC(interrupt_vector239, 239)
TRAPHANDLER_NOEC(interrupt_vector240, 240)
TRAPHANDLER_NOEC(interrupt_vector241, 241)
TRAPHANDLER_NOEC(interrupt_vector242, 242)
TRAPHANDLER_NOEC(interrupt_vector243, 243)
TRAPHANDLER_NOEC(interrupt_vector244, 244)
TRAPHANDLER_NOEC(interrupt_vector245, 245)
TRAPHANDLER_NOEC(interrupt_vector246, 246)
TRAPHANDLER_NOEC(interrupt_vector247, 247)
TRAPHANDLER_NOEC(interrupt_vector248, 248)
TRAPHANDLER_NOEC(interrupt_vector249, 249)
TRAPHANDLER_NOEC(interrupt_vector250, 250)
TRAPHANDLER_NOEC(interrupt_vector251, 251)
TRAPHANDLER_NOEC(interrupt_vector252, 252)
TRAPHANDLER_NOEC(interrupt_vector253, 253)
TRAPHANDLER_NOEC(interrupt_vector254, 254)
TRAPHANDLER_NOEC(interrupt_vector255, 255)




.data
.globl trap_handlers
trap_handlers:
	.long interrupt_vector0
	.long interrupt_vector1
	.long interrupt_vector2
	.long interrupt_vector3
	.long interrupt_vector4
	.long interrupt_vector5
	.long interrupt_vector6
	.long interrupt_vector7
	.long interrupt_vector8
	.long interrupt_vector9
	.long interrupt_vector10
	.long interrupt_vector11
	.long interrupt_vector12
	.long interrupt_vector13
	.long interrupt_vector14
	.long interrupt_vector15
	.long interrupt_vector16
	.long interrupt_vector17
	.long interrupt_vector18
	.long interrupt_vector19
	.long interrupt_vector20
	.long interrupt_vector21
	.long interrupt_vector22
	.long interrupt_vector23
	.long interrupt_vector24
	.long interrupt_vector25
	.long interrupt_vector26
	.long interrupt_vector27
	.long interrupt_vector28
	.long interrupt_vector29
	.long interrupt_vector30
	.long interrupt_vector31
	.long interrupt_vector32
	.long interrupt_vector33
	.long interrupt_vector34
	.long interrupt_vector35
	.long interrupt_vector36
	.long interrupt_vector37
	.long interrupt_vector38
	.long interrupt_vector39
	.long interrupt_vector40
	.long interrupt_vector41
	.long interrupt_vector42
	.long interrupt_vector43
	.long interrupt_vector44
	.long interrupt_vector45
	.long interrupt_vector46
	.long interrupt_vector47
	.long interrupt_vector48
	.long interrupt_vector49
	.long interrupt_vector50
	.long interrupt_vector51
	.long interrupt_vector52
	.long interrupt_vector53
	.long interrupt_vector54
	.long interrupt_vector55
	.long interrupt_vector56
	.long interrupt_vector57
	.long interrupt_vector58
	.long interrupt_vector59
	.long interrupt_vector60
	.long interrupt_vector61
	.long interrupt_vector62
	.long interrupt_vector63
	.long interrupt_vector64
	.long interrupt_vector65
	.long interrupt_vector66
	.long interrupt_vector67
	.long interrupt_vector68
	.long interrupt_vector69
	.long interrupt_vector70
	.long interrupt_vector71
	.long interrupt_vector72
	.long interrupt_vector73
	.long interrupt_vector74
	.long interrupt_vector75
	.long interrupt_vector76
	.long interrupt_vector77
	.long interrupt_vector78
	.long interrupt_vector79
	.long interrupt_vector80
	.long interrupt_vector81
	.long interrupt_vector82
	.long interrupt_vector83
	.long interrupt_vector84
	.long interrupt_vector85
	.long interrupt_vector86
	.long interrupt_vector87
	.long interrupt_vector88
	.long interrupt_vector89
	.long interrupt_vector90
	.long interrupt_vector91
	.long interrupt_vector92
	.long interrupt_vector93
	.long interrupt_vector94
	.long interrupt_vector95
	.long interrupt_vector96
	.long interrupt_vector97
	.long interrupt_vector98
	.long interrupt_vector99
	.long interrupt_vector100
	.long interrupt_vector101
	.long interrupt_vector102
	.long interrupt_vector103
	.long interrupt_vector104
	.long interrupt_vector105
	.long interrupt_vector106
	.long interrupt_vector107
	.long interrupt_vector108
	.long interrupt_vector109
	.long interrupt_vector110
	.long interrupt_vector111
	.long interrupt_vector112
	.long interrupt_vector113
	.long interrupt_vector114
	.long interrupt_vector115
	.long interrupt_vector116
	.long interrupt_vector117
	.long interrupt_vector118
	.long interrupt_vector119
	.long interrupt_vector120
	.long interrupt_vector121
	.long interrupt_vector122
	.long interrupt_vector123
	.long interrupt_vector124
	.long interrupt_vector125
	.long interrupt_vector126
	.long interrupt_vector127
	.long interrupt_vector128
	.long interrupt_vector129
	.long interrupt_vector130
	.long interrupt_vector131
	.long interrupt_vector132
	.long interrupt_vector133
	.long interrupt_vector134
	.long interrupt_vector135
	.long interrupt_vector136
	.long interrupt_vector137
	.long interrupt_vector138
	.long interrupt_vector139
	.long interrupt_vector140
	.long interrupt_vector141
	.long interrupt_vector142
	.long interrupt_vector143
	.long interrupt_vector144
	.long interrupt_vector145
	.long interrupt_vector146
	.long interrupt_vector147
	.long interrupt_vector148
	.long interrupt_vector149
	.long interrupt_vector150
	.long interrupt_vector151
	.long interrupt_vector152
	.long interrupt_vector153
	.long interrupt_vector154
	.long interrupt_vector155
	.long interrupt_vector156
	.long interrupt_vector157
	.long interrupt_vector158
	.long interrupt_vector159
	.long interrupt_vector160
	.long interrupt_vector161
	.long interrupt_vector162
	.long interrupt_vector163
	.long interrupt_vector164
	.long interrupt_vector165
	.long interrupt_vector166
	.long interrupt_vector167
	.long interrupt_vector168
	.long interrupt_vector169
	.long interrupt_vector170
	.long interrupt_vector171
	.long interrupt_vector172
	.long interrupt_vector173
	.long interrupt_vector174
	.long interrupt_vector175
	.long interrupt_vector176
	.long interrupt_vector177
	.long interrupt_vector178
	.long interrupt_vector179
	.long interrupt_vector180
	.long interrupt_vector181
	.long interrupt_vector182
	.long interrupt_vector183
	.long interrupt_vector184
	.long interrupt_vector185
	.long interrupt_vector186
	.long interrupt_vector187
	.long interrupt_vector188
	.long interrupt_vector189
	.long interrupt_vector190
	.long interrupt_vector191
	.long interrupt_vector192
	.long interrupt_vector193
	.long interrupt_vector194
	.long interrupt_vector195
	.long interrupt_vector196
	.long interrupt_vector197
	.long interrupt_vector198
	.long interrupt_vector199
	.long interrupt_vector200
	.long interrupt_vector201
	.long interrupt_vector202
	.long interrupt_vector203
	.long interrupt_vector204
	.long interrupt_vector205
	.long interrupt_vector206
	.long interrupt_vector207
	.long interrupt_vector208
	.long interrupt_vector209
	.long interrupt_vector210
	.long interrupt_vector211
	.long interrupt_vector212
	.long interrupt_vector213
	.long interrupt_vector214
	.long interrupt_vector215
	.long interrupt_vector216
	.long interrupt_vector217
	.long interrupt_vector218
	.long interrupt_vector219
	.long interrupt_vector220
	.long interrupt_vector221
	.long interrupt_vector222
	.long interrupt_vector223
	.long interrupt_vector224
	.long interrupt_vector225
	.long interrupt_vector226
	.long interrupt_vector227
	.long interrupt_vector228
	.long interrupt_vector229
	.long interrupt_vector230
	.long interrupt_vector231
	.long interrupt_vector232
	.long interrupt_vector233
	.long interrupt_vector234
	.long interrupt_vector235
	.long interrupt_vector236
	.long interrupt_vector237
	.long interrupt_vector238
	.long interrupt_vector239
	.long interrupt_vector240
	.long interrupt_vector241
	.long interrupt_vector242
	.long interrupt_vector243
	.long interrupt_vector244
	.long interrupt_vector245
	.long interrupt_vector246
	.long interrupt_vector247
	.long interrupt_vector248
	.long interrupt_vector249
	.long interrupt_vector250
	.long interrupt_vector251
	.long interrupt_vector252
	.long interrupt_vector253
	.long interrupt_vector254
	.long interrupt_vector255

/*
 * Lab 3: Your code here for _alltraps
 */
.text
_alltraps:
	// push values to make the stack look like a struct Trapframe
	pushl %ds			# -> Data Segment
	pushl %es			# -> Extra Segment
	pushal				# Push EAX, ECX, EDX, EBX, original ESP, EBP, ESI, EDI
	//load GD_KD into %ds and %es
	movw $GD_KD, %ax
	movw %ax, %es
	movw %ax, %ds
	//pushl %esp to pass a pointer to the Trapframe as an argument to trap()
	pushl %esp
	call trap
