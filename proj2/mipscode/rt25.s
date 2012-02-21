.text
.set noat

.ent _start
.global _start
_start:

  ori $t0, $0, 1
  ori $t1, $0, 0
  slti $t2, $t0, 0
  slti $t2, $t1, 1

  slti $t2, $t0, 1
  slti $t2, $t1, 1

  ori $t0, $0, 1
  negu $t0, $t0
  ori $t1, $0, 0
  slti $t2, $t0, 0
  slti $t2, $t1, -1
  slti $t2, $t0, -1
  slti $t2, $t0, -32768

  ori $v0, $0, 10
  syscall

.end _start
