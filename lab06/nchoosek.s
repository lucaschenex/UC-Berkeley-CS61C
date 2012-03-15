main:
	li      $a0, 4
	li      $a1, 0
	jal     nchoosek                # evaluate C(4, 0); should be 1
	move    $a0, $v0
	jal     printint

	li      $a0, 4
	li      $a1, 1
	jal     nchoosek                # evaluate C(4, 1); should be 4
	move    $a0, $v0
	jal     printint

	li      $a0, 4
	li      $a1, 2
	jal     nchoosek                # evaluate C(4, 2); should be 6
	move    $a0, $v0
	jal     printint

	li      $a0, 4
	li      $a1, 3
	jal     nchoosek                # evaluate C(4, 3); should be 4
	move    $a0, $v0
	jal     printint

	li      $a0, 4
	li      $a1, 4
	jal     nchoosek                # evaluate C(4, 4); should be 1
	move    $a0, $v0
	jal     printint

	li      $a0, 4
	li      $a1, 5
	jal     nchoosek                # evaluate C(4, 5); should be 0
	move    $a0, $v0
	jal     printint

	li      $v0, 10
	syscall

nchoosek:

	# you fill in the prologue
	addiu   $sp, $sp, -16		# move stack pointer
	sw      $ra, 0($sp)		# store return address
	sw      $s2, 4($sp)		# saved temp2
	sw      $s1, 8($sp)		# saved temp1
	sw      $s0, 12($sp)		# saved temp0

	beq     $a1, $0, return1
	beq     $a0, $a1, return1
	beq     $a0, $0, return0
	blt     $a0, $a1, return0
	
	addiu   $a0, $a0, -1              # C(n, k) = C(n-1, k) + C(n-1, k-1)
	move    $s0, $a0
	move    $s1, $a1
	jal     nchoosek
	move    $s2, $v0
	move    $a0, $s0
	addiu   $a1, $s1, -1
	jal     nchoosek
	addu    $v0, $v0, $s2
	j       return
return0:
	li      $v0, 0
	j       return
return1:
	li      $v0, 1
return:
	# you fill in the epilogue
	lw	$s0, 12($sp)		# restore save temp2
	lw      $s1, 8($sp)		# save temp1
	lw      $s2, 4($sp)		# save temp0
	lw      $ra, 0($sp)		# restore return address
	addiu   $sp, $sp, 16		# release stack memory
	jr	$ra			# return to main


printint:
	addiu   $sp, $sp, -4
	sw      $ra, 0($sp)
	li      $v0, 1
	syscall
	li      $a0, '\n'
	li      $v0, 11
	syscall
	lw      $ra, 0($sp)
	addiu   $sp, $sp, 4
	jr      $ra
