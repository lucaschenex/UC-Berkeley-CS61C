.text
.set noat

.global _start
.ent    _start 
_start:
  
  la $t0, x

  lh $t1, 0($t0)
  lh $t1, 2($t0)

  addiu $t0, $t0, 4

  lh $t1, -4($t0)
  lh $t1, -2($t0)

  li $v0, 10
  syscall

_end:

.end _start

.data
x: .word 0x01234567
