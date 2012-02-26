.text

# _start is the entry point into any program.
.global _start
.ent    _start 
_start:

#
#  The header ends here, and code goes below
#
	ori   $a0, $zero, 25
	ori   $v0, $zero, 1
	syscall
	ori   $a0, %lo(str)
	ori   $v0, $zero, 4
	syscall

# exit the simulation (v0 = 10, syscall)
	ori   $v0, $zero, 10
	syscall

.end _start

.data
newline: .asciiz " \n"
str: .asciiz "Hello, world!\n"
