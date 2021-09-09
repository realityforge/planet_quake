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
	dpps	$0x71, %xmm0, %xmm0
	movss	%xmm0, 8(%esp)
	flds	8(%esp)

	popl	%eax
	ret

.globl C(VectorNormalizeFast)
C(VectorNormalizeFast):

	pushl		%eax

	movl		8(%esp), %eax
	movups		(%eax), %xmm0
	movaps		%xmm0, %xmm1
	dpps		$0x71, %xmm1, %xmm1
	rsqrtss		%xmm1, %xmm1
	movlhps		%xmm1, %xmm1
	movsldup	%xmm1, %xmm1
	mulps		%xmm1, %xmm0
	movlps		%xmm0, (%eax)
	pextrd		$2, %xmm0, 8(%eax)

	popl		%eax
	ret

.globl C(VectorLength)
C(VectorLength):

	pushl	%eax

	movl	8(%esp), %eax
	movups	(%eax), %xmm0
	dpps	$0x71, %xmm0, %xmm0
	ucomiss	zero, %xmm0
	je	done0
	movss	%xmm0, %xmm1
	rsqrtss	%xmm1, %xmm1
	mulss	%xmm1, %xmm0
	movss	%xmm0, 8(%esp)
	flds	8(%esp)

	popl	%eax
	ret

.globl C(VectorNormalize)
C(VectorNormalize):

	pushl		%eax

	movl		8(%esp), %eax
	movups		(%eax), %xmm0
	movaps		%xmm0, %xmm1
	dpps		$0x71, %xmm1, %xmm1
	ucomiss		zero, %xmm1
	je		done0
	ucomiss		one, %xmm1
	je		done1
	pslldq		$4, %xmm0
	movss		%xmm1, %xmm0
	rsqrtss		%xmm1, %xmm1
	movlhps		%xmm1, %xmm1
	movsldup	%xmm1, %xmm1
	mulps		%xmm1, %xmm0
	movss		%xmm0, 8(%esp)
	flds		8(%esp)
	pextrd		$1, %xmm0, (%eax)
	movhps		%xmm0, 4(%eax)

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
	dpps		$0x71, %xmm1, %xmm1
	movl		12(%esp), %eax
	ucomiss		zero, %xmm1
	je		clear0
	ucomiss		one, %xmm1
	je		clear1
	pslldq		$4, %xmm0
	movss		%xmm1, %xmm0
	rsqrtss		%xmm1, %xmm1
	movlhps		%xmm1, %xmm1
	movsldup	%xmm1, %xmm1
	mulps		%xmm1, %xmm0
	movss		%xmm0, 8(%esp)
	flds		8(%esp)
	pextrd		$1, %xmm0, (%eax)
	movhps		%xmm0, 4(%eax)

	popl		%eax
	ret

clear0:
	flds		zero
	movss		%xmm1, (%eax)
	movss		%xmm1, 4(%eax)
	movss		%xmm1, 8(%eax)

	popl		%eax
	ret

clear1:
	flds		one
	movlps		%xmm0, (%eax)
	pextrd		$2, %xmm0, 8(%eax)

	popl		%eax
	ret
