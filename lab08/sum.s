	.file	"sum.c"
	.text
	.p2align 4,,15
	.globl	sum_naive
	.type	sum_naive, @function
sum_naive:
.LFB532:
	.cfi_startproc
	xorl	%eax, %eax
	testl	%edi, %edi
	jle	.L2
	xorl	%edx, %edx
	.p2align 4,,10
	.p2align 3
.L3:
	addl	(%rsi,%rdx,4), %eax
	addq	$1, %rdx
	cmpl	%edx, %edi
	jg	.L3
.L2:
	rep
	ret
	.cfi_endproc
.LFE532:
	.size	sum_naive, .-sum_naive
	.p2align 4,,15
	.globl	sum_unrolled
	.type	sum_unrolled, @function
sum_unrolled:
.LFB533:
	.cfi_startproc
	leal	3(%rdi), %r8d
	testl	%edi, %edi
	cmovns	%edi, %r8d
	xorl	%eax, %eax
	andl	$-4, %r8d
	jle	.L8
	leal	-1(%r8), %ecx
	xorl	%edx, %edx
	shrl	$2, %ecx
	addq	$1, %rcx
	salq	$4, %rcx
	.p2align 4,,10
	.p2align 3
.L9:
	addl	(%rsi,%rdx), %eax
	addl	4(%rsi,%rdx), %eax
	addl	8(%rsi,%rdx), %eax
	addl	12(%rsi,%rdx), %eax
	addq	$16, %rdx
	cmpq	%rcx, %rdx
	jne	.L9
.L8:
	cmpl	%r8d, %edi
	jle	.L10
	subl	$1, %edi
	xorl	%edx, %edx
	subl	%r8d, %edi
	movslq	%r8d, %r8
	leaq	4(,%rdi,4), %rdi
	leaq	(%rsi,%r8,4), %rcx
	.p2align 4,,10
	.p2align 3
.L11:
	addl	(%rcx,%rdx), %eax
	addq	$4, %rdx
	cmpq	%rdi, %rdx
	jne	.L11
.L10:
	rep
	ret
	.cfi_endproc
.LFE533:
	.size	sum_unrolled, .-sum_unrolled
	.p2align 4,,15
	.globl	sum_vectorized_unrolled
	.type	sum_vectorized_unrolled, @function
sum_vectorized_unrolled:
.LFB535:
	.cfi_startproc
	xorl	%eax, %eax
	ret
	.cfi_endproc
.LFE535:
	.size	sum_vectorized_unrolled, .-sum_vectorized_unrolled
	.p2align 4,,15
	.globl	sum_vectorized
	.type	sum_vectorized, @function
sum_vectorized:
.LFB534:
	.cfi_startproc
	leal	3(%rdi), %ecx
	testl	%edi, %edi
	pxor	%xmm1, %xmm1
	cmovns	%edi, %ecx
	andl	$-4, %ecx
	movdqa	%xmm1, -24(%rsp)
	jle	.L17
	leal	-1(%rcx), %eax
	shrl	$2, %eax
	mov	%eax, %r8d
	movq	%r8, %rax
	salq	$4, %rax
	leaq	16(%rsi,%rax), %rdx
	movq	%rsi, %rax
	.p2align 4,,10
	.p2align 3
.L18:
	movdqu	(%rax), %xmm0
	addq	$16, %rax
	cmpq	%rdx, %rax
	paddd	-24(%rsp), %xmm0
	jne	.L18
	leaq	1(%r8), %rax
	salq	$4, %rax
	addq	%rax, %rsi
.L17:
	movl	(%rsi), %eax
	addl	4(%rsi), %eax
	addl	8(%rsi), %eax
	addl	12(%rsi), %eax
	cmpl	%ecx, %edi
	movdqu	%xmm0, -24(%rsp)
	jle	.L19
	subl	$1, %edi
	xorl	%edx, %edx
	subl	%ecx, %edi
	movslq	%ecx, %rcx
	leaq	4(,%rdi,4), %rdi
	leaq	(%rsi,%rcx,4), %rcx
	.p2align 4,,10
	.p2align 3
.L20:
	addl	(%rcx,%rdx), %eax
	addq	$4, %rdx
	cmpq	%rdi, %rdx
	jne	.L20
.L19:
	rep
	ret
	.cfi_endproc
.LFE534:
	.size	sum_vectorized, .-sum_vectorized
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"%20s: "
.LC3:
	.string	"%.2f microseconds\n"
.LC4:
	.string	"ERROR!\n"
	.text
	.p2align 4,,15
	.globl	benchmark
	.type	benchmark, @function
benchmark:
.LFB536:
	.cfi_startproc
	movq	%rbx, -48(%rsp)
	movq	%rbp, -40(%rsp)
	movl	%edi, %ebx
	.cfi_offset 6, -48
	.cfi_offset 3, -56
	movq	%r12, -32(%rsp)
	movq	%r13, -24(%rsp)
	movq	%rsi, %rbp
	movq	%r14, -16(%rsp)
	movq	%r15, -8(%rsp)
	movq	%rdx, %r12
	.cfi_offset 15, -16
	.cfi_offset 14, -24
	.cfi_offset 13, -32
	.cfi_offset 12, -40
	subq	$88, %rsp
	.cfi_def_cfa_offset 96
	movq	%rcx, %r13
	call	*%rdx
	movl	%eax, %r14d
#APP
# 11 "sum.c" 1
	rdtsc
# 0 "" 2
#NO_APP
	movl	%edx, %r15d
	movl	%eax, 12(%rsp)
	movq	%rbp, %rsi
	movl	%ebx, %edi
	call	*%r12
	leal	(%rax,%r14), %r12d
#APP
# 11 "sum.c" 1
	rdtsc
# 0 "" 2
#NO_APP
	movl	$.LC0, %esi
	movl	%edx, 16(%rsp)
	mov	%eax, %r14d
	movq	%r13, %rdx
	xorl	%eax, %eax
	movl	$1, %edi
	call	__printf_chk
	xorl	%edx, %edx
	testl	%ebx, %ebx
	jle	.L24
	xorl	%eax, %eax
	.p2align 4,,10
	.p2align 3
.L25:
	addl	0(%rbp,%rax,4), %edx
	addq	$1, %rax
	cmpl	%eax, %ebx
	jg	.L25
	addl	%edx, %edx
.L24:
	cmpl	%edx, %r12d
	je	.L31
	movq	40(%rsp), %rbx
	movq	48(%rsp), %rbp
	movl	$.LC4, %esi
	movq	56(%rsp), %r12
	movq	64(%rsp), %r13
	movl	$1, %edi
	movq	72(%rsp), %r14
	movq	80(%rsp), %r15
	xorl	%eax, %eax
	addq	$88, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 8
	jmp	__printf_chk
	.p2align 4,,10
	.p2align 3
.L31:
	.cfi_restore_state
	mov	12(%rsp), %eax
	salq	$32, %r15
	orq	%rax, %r15
	movl	16(%rsp), %eax
	salq	$32, %rax
	orq	%r14, %rax
	subq	%r15, %rax
	js	.L27
	cvtsi2sdq	%rax, %xmm0
.L28:
	divsd	.LC1(%rip), %xmm0
	movq	40(%rsp), %rbx
	movl	$.LC3, %esi
	movq	48(%rsp), %rbp
	movq	56(%rsp), %r12
	movl	$1, %edi
	movq	64(%rsp), %r13
	movq	72(%rsp), %r14
	movl	$1, %eax
	movq	80(%rsp), %r15
	mulsd	.LC2(%rip), %xmm0
	addq	$88, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 8
	jmp	__printf_chk
.L27:
	.cfi_restore_state
	movq	%rax, %rdx
	andl	$1, %eax
	shrq	%rdx
	orq	%rax, %rdx
	cvtsi2sdq	%rdx, %xmm0
	addsd	%xmm0, %xmm0
	jmp	.L28
	.cfi_endproc
.LFE536:
	.size	benchmark, .-benchmark
	.section	.rodata.str1.1
.LC5:
	.string	"naive"
.LC6:
	.string	"unrolled"
.LC7:
	.string	"vectorized"
.LC8:
	.string	"vectorized unrolled"
	.section	.text.startup,"ax",@progbits
	.p2align 4,,15
	.globl	main
	.type	main, @function
main:
.LFB537:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	xorl	%edi, %edi
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%r12
	xorl	%r12d, %r12d
	.cfi_offset 12, -24
	pushq	%rbx
	subq	$16, %rsp
	movq	%fs:40, %rax
	movq	%rax, -24(%rbp)
	xorl	%eax, %eax
	.cfi_offset 3, -32
	call	time
	movq	%rax, %rdi
	xorl	%eax, %eax
	call	srand48
	subq	$31136, %rsp
	leaq	15(%rsp), %rbx
	andq	$-16, %rbx
	.p2align 4,,10
	.p2align 3
.L33:
	xorl	%eax, %eax
	call	lrand48
	movl	%eax, (%rbx,%r12)
	addq	$4, %r12
	cmpq	$31108, %r12
	jne	.L33
	movl	$.LC5, %ecx
	movl	$sum_naive, %edx
	movq	%rbx, %rsi
	movl	$7777, %edi
	call	benchmark
	movl	$.LC6, %ecx
	movl	$sum_unrolled, %edx
	movq	%rbx, %rsi
	movl	$7777, %edi
	call	benchmark
	movl	$.LC7, %ecx
	movl	$sum_vectorized, %edx
	movq	%rbx, %rsi
	movl	$7777, %edi
	call	benchmark
	movl	$sum_vectorized_unrolled, %edx
	movl	$.LC8, %ecx
	movq	%rbx, %rsi
	movl	$7777, %edi
	call	benchmark
	xorl	%eax, %eax
	movq	-24(%rbp), %rdx
	xorq	%fs:40, %rdx
	jne	.L36
	leaq	-16(%rbp), %rsp
	popq	%rbx
	popq	%r12
	popq	%rbp
	.cfi_remember_state
	.cfi_def_cfa 7, 8
	ret
.L36:
	.cfi_restore_state
	call	__stack_chk_fail
	.cfi_endproc
.LFE537:
	.size	main, .-main
	.section	.rodata.cst8,"aM",@progbits,8
	.align 8
.LC1:
	.long	2684354560
	.long	1105254043
	.align 8
.LC2:
	.long	0
	.long	1093567616
	.ident	"GCC: (Ubuntu/Linaro 4.6.1-9ubuntu3) 4.6.1"
	.section	.note.GNU-stack,"",@progbits
