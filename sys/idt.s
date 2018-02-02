.global lidt
  lidt:
 lidt (%rdi)
  retq

.macro PUSHALLREGS
    pushq %rdi
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rbp
    pushq %rsi
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
.endm

.macro POPALLREGS
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rsi
    popq %rbp
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax
    popq %rdi
.endm

.text

.extern interrupt_handler
.extern timer_irq_handler
.extern iservice_handler

.global int0
.global int1
.global int_common

.global iservice0
.global iservice10
.global iservice13
.global iservice14
.global iservice_common


int_common:
	PUSHALLREGS
	movq %rsp, %rdi
	callq interrupt_handler
	POPALLREGS
	add $0x10, %rsp
	sti
	iretq

int1:
	cli
	pushq $0x0
	pushq $0x21
	jmp int_common


int0:
	cli
	PUSHALLREGS
	movq %rsp, %rdi
#	callq timer_irq_handler
	POPALLREGS
	sti
	iretq

iservice0:
	cli
	pushq $0
	pushq $0
	jmp iservice_common

iservice10:
	cli
	pushq $0
	pushq $10
	jmp iservice_common	

iservice14:
	cli
	pushq $0
	pushq $14
	jmp iservice_common

iservice13:
	cli
	pushq $0
	pushq $13
	jmp iservice_common	

# writing iservice common separately so that without ambiguity can call iservice handler explicitly
# can merge also ..... need to look.. how to do	

iservice_common:
	PUSHALLREGS
	movq %rsp, %rdi
	callq iservice_handler
	POPALLREGS
	add $0x10, %rsp
	sti
	iretq

