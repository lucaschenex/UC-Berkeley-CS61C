.text

# _start is the entry point into any program.
.global _start
.ent    _start 
_start:

#
#  The header ends here, and code goes below
#
	# print sll text
	ori   $a0, $zero, sll
	ori   $v0, $zero, 4
	syscall
	# execute sll
	ori   $t0, $zero, 2
	ori   $t1, $zero, 2
	sll   $a0, $t0, $t1
	ori   $v0, $zero, 1
	syscall
	ori   $a0, $zero, newline
	ori   $v0, $zero, 4
	syscall

	# print srl text
	ori   $a0, $zero, srl
	ori   $v0, $zero, 4
	syscall
	# execute srl
	ori   $t0, $zero, 16
	ori   $t1, $zero, 1
	srl   $a0, $t0, $t1
	ori   $v0, $zero, 1
	syscall
	ori   $a0, $zero, newline
	ori   $v0, $zero, 4
	syscall

	# print sra text
	ori   $a0, $zero, sra
	ori   $v0, $zero, 4
	syscall
	# execute sra
	ori   $t0, $zero, 16
	subu  $t0, $zero, $t0
	ori   $t1, $zero, 1
	sra   $a0, $t0, $t1
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
sll:	.asciiz "sll: 2 << 2 =  "
srl:	.asciiz "srl: 16 >> 1 = "
sra:	.asciiz "sra: (-16) >> 1 = "
