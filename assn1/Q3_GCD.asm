.data

str0:
  .asciiz "Enter a 32-bit unsigned integer: "
str1:
  .asciiz "The GCD of the two integers is: "
newline:
  .asciiz "\n"

.text

################################################################################
# gcd
#
# INPUTs
#   $a0: a 32-bit unsigned integer
#   $a1: a 32-bit unsigned integer
#
# OUTPUTs
#   $v0: the GCD of the two unsigned integers
#
gcd:
  # stack.push($ra) <-- modify this part depending w.r.t. your implementation
  addi $sp, $sp, -12
  sw $ra, 8($sp)
  sw $s0, 4($sp)
  sw $s1, 0($sp)

  # FIXME
  beq $a1, $zero, L1
  move $t0, $a1
  div $a0, $a1
  mfhi $a1
  move $a0, $t0
  jal gcd
  b exit
L1:
  move $v0, $a0
exit :

  # $ra = stack.pop() <-- modify this part depending w.r.t. your implementation
  lw $ra, 8($sp)
  lw $s0, 4($sp)
  lw $s1, 0($sp)
  addi $sp, $sp, 12

  jr $ra
################################################################################

.globl main
main:

  # stack.push($ra)
  # stack.push($s0)
  # stack.push($s1)
  addi $sp, $sp, -12
  sw $ra, 0($sp)
  sw $s0, 4($sp)
  sw $s1, 8($sp)

  # $s0 = read_int
  li $v0, 4
  la $a0, str0
  syscall
  li $v0, 5
  syscall
  move $s0, $v0

  # $s1 = read_int
  li $v0, 4
  la $a0, str0
  syscall
  li $v0, 5
  syscall
  move $s1, $v0

  # $t0 = gcd($s0, $s1)
  move $a0, $s0
  move $a1, $s1
  jal gcd
  move $t0, $v0

  # print_string(str1)
  li $v0, 4
  la $a0, str1
  syscall
  # print_int($t0)
  li $v0, 1
  move $a0, $t0
  syscall
  # print_string(newline)
  li $v0, 4
  la $a0, newline
  syscall

  # stack.pop($s1)
  # stack.pop($s0)
  # stack.pop($ra)
  lw $ra, 0($sp)
  lw $s0, 4($sp)
  lw $s1, 8($sp)
  addi $sp, $sp, 12

  # return
  jr $ra

