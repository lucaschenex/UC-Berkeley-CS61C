.text

# _start is the entry point into any program.
.global _start
.ent    _start 
_start:

#
#  The header ends here, and code goes below
#
	# print slt text
	ori   $a0, $zero, slt
	ori   $v0, $zero, 4
	syscall
	# execute slt
	ori   $t0, $zero, 1
	ori   $t1, $zero, 2
	slt   $a0, $t0, $t1
	ori   $v0, $zero, 1
	syscall
	ori   $a0, $zero, newline
	ori   $v0, $zero, 4
	syscall

	# print slt text
	ori   $a0, $zero, slt2
	ori   $v0, $zero, 4
	syscall
	# execute slt
	ori   $t0, $zero, 1
	subu  $t0, $zero, $t0
	ori   $t1, $zero, 0
	slt   $a0, $t0, $t1
	ori   $v0, $zero, 1
	syscall
	ori   $a0, $zero, newline
	ori   $v0, $zero, 4
	syscall

	# print sltu text
	ori   $a0, $zero, sltu
	ori   $v0, $zero, 4
	syscall
	# execute sra
	ori   $t0, $zero, 1
	ori   $t1, $zero, 2
	sltu   $a0, $t0, $t1
	ori   $v0, $zero, 1
	syscall
	ori   $a0, $zero, newline
	ori   $v0, $zero, 4
	syscall

	# print sltu text
	ori   $a0, $zero, sltu2
	ori   $v0, $zero, 4
	syscall
	# execute sra
	ori   $t0, $zero, 1
	subu  $t0, $zero, $t0
	ori   $t1, $zero, 0
	sltu   $a0, $t0, $t1
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
slt:	.asciiz "slt: (1 < 2) =  "
slt2:	.asciiz "slt: (-1 < 0) =  "
sltu:	.asciiz "sltu: (1 < 2) = "
sltu2:	.asciiz "sltu: (-1 < 0) = "
