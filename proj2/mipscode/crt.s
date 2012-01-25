# This is the entry point for simple C programs.
# _start just initializes the stack pointer and calls main.

.text

.global _start
.ent    _start 
_start:

# set up stack
	la    $sp, stack_top

# argc = 0, argv = NULL
	li    $a0, 0
	li    $a1, 0

# call main()
	jal   main

# exit the simulation
	jal   mips_exit

.end    _start

.global mips_print_string
.ent    mips_print_string
mips_print_string:
	li    $v0, 4
	syscall
	jr    $ra
.end    mips_print_string

.global mips_print_char
.ent    mips_print_char
mips_print_char:
	li    $v0, 11
	syscall
	jr    $ra
.end    mips_print_char

.global mips_print_int
.ent    mips_print_int
mips_print_int:
	li    $v0, 1
	syscall
	jr    $ra
.end    mips_print_int

.global mips_exit
.ent    mips_exit
mips_exit:
	li    $v0, 10
	syscall
	jr    $ra
.end    mips_exit

.bss
stack_bot: .skip 65536
stack_top:
