.text

# _start is the entry point into any program.
.global _start
.ent    _start 
_start:

#
#  The header ends here, and code goes below
#
	# Jump and link register
	jal	jump

	# Jump register
	j 	end
	
end:
	lw      $ra, 0($sp)
	addu   $sp, $sp, 4
	ori   $a0, $zero, j
	ori   $v0, $zero, 4
	syscall
	ori   $v0, $zero, 10
	syscall	

jump:
	ori	$t0, $zero, 4
	subu 	$sp, $zero, $t0
	sw      $ra, 0($sp)

	ori   $a0, $zero, jal
	ori   $v0, $zero, 4
	syscall
	j    	$ra
	
.end _start

.data
newline: .asciiz " \n"
jal:	.asciiz "Jumped and linked"
j:	.asciiz "Jumped"
	
