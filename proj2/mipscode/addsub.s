.text

# _start is the entry point into any program.
.global _start
.ent    _start 
_start:

#
#  The header ends here, and code goes below
#
	# print addu text
	ori   $a0, $zero, addu
	ori   $v0, $zero, 4
	syscall
	# execute addu
	ori   $t0, $zero, 1
	ori   $t1, $zero, 15
	addu  $a0, $t0, $t1
	ori   $v0, $zero, 1
	syscall
	ori   $a0, $zero, newline
	ori   $v0, $zero, 4
	syscall

	# print subu text
	ori   $a0, $zero, subu
	ori   $v0, $zero, 4
	syscall
	# execute subu
	ori   $t0, $zero, 1
	ori   $t1, $zero, 15
	subu  $a0, $t0, $t1
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
addu:	.asciiz "addu: 1 + 15 = "
subu:	.asciiz "subu: 1 - 15 = "
