#include "qasm.h"

.data

.align 4
zero:	.single	0.0
one:	.single	1.0

.text

.globl C(Q_rsqrt)
C(Q_rsqrt):

	rsqrtss	4(%esp), %xmm0
	movss	%xmm0, 4(%esp)
	flds	4(%esp)
	ret

.globl C(SquareRootFloat)
C(SquareRootFloat):

	movss	4(%esp), %xmm0
	movss	%xmm0, %xmm1
	rsqrtss	%xmm1, %xmm1
	mulss	%xmm1, %xmm0
	movss	%xmm0, 4(%esp)
	flds	4(%esp)
	ret

.globl C(VectorLengthSquared)
C(VectorLengthSquared):

	pushl	%eax

	movl	8(%esp), %eax
	movups	(%eax), %xmm0
	mulps	%xmm0, %xmm0
	movss	%xmm0, %xmm1
	psrldq	$4, %xmm0
	addss	%xmm0, %xmm1
	psrldq	$4, %xmm0
	addss	%xmm0, %xmm1
	movss	%xmm1, 8(%esp)
	flds	8(%esp)

	popl	%eax
	ret

.globl C(VectorNormalizeFast)
C(VectorNormalizeFast):

	pushl		%eax

	movl		8(%esp), %eax
	movups		(%eax), %xmm0
	movaps		%xmm0, %xmm1
	mulps		%xmm1, %xmm1
	movss		%xmm1, %xmm2
	psrldq		$4, %xmm1
	addss		%xmm1, %xmm2
	psrldq		$4, %xmm1
	addss		%xmm1, %xmm2
	rsqrtss		%xmm2, %xmm2
	movlhps		%xmm2, %xmm2
	movsldup	%xmm2, %xmm2
	mulps		%xmm2, %xmm0
	movlps		%xmm0, (%eax)
	psrldq		$8, %xmm0
	movss		%xmm0, 8(%eax)

	popl		%eax
	ret

.globl C(VectorLength)
C(VectorLength):

	pushl	%eax

	movl	8(%esp), %eax
	movups	(%eax), %xmm0
	mulps	%xmm0, %xmm0
	movss	%xmm0, %xmm1
	psrldq	$4, %xmm0
	addss	%xmm0, %xmm1
	psrldq	$4, %xmm0
	addss	%xmm0, %xmm1
	ucomiss	zero, %xmm1
	je	done0
	movss	%xmm1, %xmm0
	rsqrtss	%xmm0, %xmm0
	mulss	%xmm0, %xmm1
	movss	%xmm1, 8(%esp)
	flds	8(%esp)

	popl	%eax
	ret

.globl C(VectorNormalize)
C(VectorNormalize):

	pushl		%eax

	movl		8(%esp), %eax
	movups		(%eax), %xmm0
	movaps		%xmm0, %xmm1
	mulps		%xmm1, %xmm1
	movss		%xmm1, %xmm2
	psrldq		$4, %xmm1
	addss		%xmm1, %xmm2
	psrldq		$4, %xmm1
	addss		%xmm1, %xmm2
	ucomiss		zero, %xmm2
	je		done0
	ucomiss		one, %xmm2
	je		done1
	pslldq		$4, %xmm0
	movss		%xmm2, %xmm0
	rsqrtss		%xmm2, %xmm2
	movlhps		%xmm2, %xmm2
	movsldup	%xmm2, %xmm2
	mulps		%xmm2, %xmm0
	movss		%xmm0, 8(%esp)
	flds		8(%esp)
	movhps		%xmm0, 4(%eax)
	psrldq		$4, %xmm0
	movss		%xmm0, (%eax)

	popl		%eax
	ret

done0:
	flds		zero
	popl		%eax
	ret

done1:
	flds		one
	popl		%eax
	ret

.globl C(VectorNormalize2)
C(VectorNormalize2):

	pushl		%eax

	movl		8(%esp), %eax
	movups		(%eax), %xmm0
	movaps		%xmm0, %xmm1
	mulps		%xmm1, %xmm1
	movss		%xmm1, %xmm2
	psrldq		$4, %xmm1
	addss		%xmm1, %xmm2
	psrldq		$4, %xmm1
	addss		%xmm1, %xmm2
	movl		12(%esp), %eax
	ucomiss		zero, %xmm2
	je		clear0
	ucomiss		one, %xmm2
	je		clear1
	pslldq		$4, %xmm0
	movss		%xmm2, %xmm0
	rsqrtss		%xmm2, %xmm2
	movlhps		%xmm2, %xmm2
	movsldup	%xmm2, %xmm2
	mulps		%xmm2, %xmm0
	movss		%xmm0, 8(%esp)
	flds		8(%esp)
	movhps		%xmm0, 4(%eax)
	psrldq		$4, %xmm0
	movss		%xmm0, (%eax)

	popl		%eax
	ret

clear0:
	flds		zero
	movss		%xmm2, (%eax)
	movss		%xmm2, 4(%eax)
	movss		%xmm2, 8(%eax)

	popl		%eax
	ret

clear1:
	flds		one
	movlps		%xmm0, (%eax)
	psrldq		$8, %xmm0
	movss		%xmm0, 8(%eax)

	popl		%eax
	ret
