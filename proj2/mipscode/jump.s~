.text

# _start is the entry point into any program.
.global _start
.ent    _start 
_start:

#
#  The header ends here, and code goes below
#
	# Jump and link register
	la	$s0, jump
	jalr	$s0, $s1

	# Jump register
	la 	$s0, end
	jr 	$s0
	
end:
	ori   $a0, $zero, jr
	ori   $v0, $zero, 4
	syscall
	ori   $v0, $zero, 10
	syscall	

jump:
	ori   $a0, $zero, jalr
	ori   $v0, $zero, 4
	syscall
	jr    	$s1
	
.end _start

.data
newline: .asciiz " \n"
jalr:	.asciiz "Jumped and linked"
jr:	.asciiz "Jumped"
	
