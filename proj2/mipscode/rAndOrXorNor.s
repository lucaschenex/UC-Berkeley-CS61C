.text

# _start is the entry point into any program.
.global _start
.ent    _start 
_start:

#
#  The header ends here, and code goes below
#
	# print and text
	ori   $a0, $zero, and
	ori   $v0, $zero, 4
	syscall
	# execute and
	ori   $t0, $zero, 6
	ori   $t1, $zero, 2
	and   $a0, $t0, $t1
	ori   $v0, $zero, 1
	syscall
	ori   $a0, $zero, newline
	ori   $v0, $zero, 4
	syscall

	# print or text
	ori   $a0, $zero, or
	ori   $v0, $zero, 4
	syscall
	# execute or
	ori   $t0, $zero, 9
	ori   $t1, $zero, 4
	or    $a0, $t0, $t1
	ori   $v0, $zero, 1
	syscall
	ori   $a0, $zero, newline
	ori   $v0, $zero, 4
	syscall

	# print xor text
	ori   $a0, $zero, xor
	ori   $v0, $zero, 4
	syscall
	# execute and
	ori   $t0, $zero, 12
	ori   $t1, $zero, 6
	xor   $a0, $t0, $t1
	ori   $v0, $zero, 1
	syscall
	ori   $a0, $zero, newline
	ori   $v0, $zero, 4
	syscall

	# print nor text
	ori   $a0, $zero, nor
	ori   $v0, $zero, 4
	syscall
	# execute or
	ori   $t0, $zero, 9
	ori   $t1, $zero, 4
	nor   $a0, $t0, $t1
	ori   $v0, $zero, 1
	syscall
	ori   $a0, $zero, newline
	ori   $v0, $zero, 4
	syscall
	
# exit the simulation (v0 = 10, syscall)
	ori   $v0, $zero, 10
	syscall

.end _start

.data
newline: .asciiz " \n"
and:	.asciiz "and: 0110 & 0010 = "
or:	.asciiz "or: 1001 | 0100  = "
xor:	.asciiz "xor: 1100 ^ 0110 = "
nor:	.asciiz "nor: ~(1001 | 0100) = "
