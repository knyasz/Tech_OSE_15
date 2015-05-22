/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#include <kern/picirq.h>


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
 *
 * Have been updated for challenge 1
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	.text;								\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps;							\
	.data;								\
	.long name;

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 *
 * Have been updated for challenge 1
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	.text;								\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps;							\
	.data;								\
	.long name;



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

.data
.globl trap_handlers
trap_handlers:
TRAPHANDLER_NOEC(interrupt_vector0, T_DIVIDE)
TRAPHANDLER_NOEC(interrupt_vector1, T_DEBUG)
TRAPHANDLER_NOEC(interrupt_vector2, T_NMI)
TRAPHANDLER_NOEC(interrupt_vector3, T_BRKPT)
TRAPHANDLER_NOEC(interrupt_vector4, T_OFLOW)
TRAPHANDLER_NOEC(interrupt_vector5, T_BOUND)
TRAPHANDLER_NOEC(interrupt_vector6, T_ILLOP)
TRAPHANDLER_NOEC(interrupt_vector7, T_DEVICE)
TRAPHANDLER(interrupt_vector8, T_DBLFLT)
TRAPHANDLER_NOEC(interrupt_vector9, 9)	/* Reserved */
TRAPHANDLER(interrupt_vector10, T_TSS)
TRAPHANDLER(interrupt_vector11, T_SEGNP)
TRAPHANDLER(interrupt_vector12, T_STACK)
TRAPHANDLER(interrupt_vector13, T_GPFLT)
TRAPHANDLER(interrupt_vector14, T_PGFLT)
TRAPHANDLER_NOEC(interrupt_vector15, 15)		/* Reserved */
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




/*
 * Lab 3: Your code here for _alltraps
 */
.text
.globl _alltraps
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



