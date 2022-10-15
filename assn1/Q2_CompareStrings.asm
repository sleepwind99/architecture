.data

str0:
  .asciiz "TestString0"
str1:
  .asciiz "TestString"

log_str0:
  .asciiz "INFO: compareStrings returned: "
log_newline:
  .asciiz "\n"

.text

################################################################################
# compareStrings
#
# INPUTs
#   $a0: the starting memory address of str0
#   $a1: the starting memory address of str1
#
# OUTPUTs
#   $v0: a 32-bit signed integer indicating the comparison result
#
compareStrings:
  # stack.push($ra) <-- modify this part depending w.r.t. your implementation
  addi $sp, $sp, -12
  sw $ra, 8($sp)
  sw $s0, 4($sp)
  sw $s1, 0($sp)

  # FIXME
  li $t2, -1
  move $s0, $a0
  move $s1, $a1
loop:
  lb $t0, 0($s0)
  lb $t1, 0($s1)
  beq $t0, $zero, L3
  beq $t1, $zero, L4
  bne $t0, $t1, L1
  addi $s0, $s0, 1
  addi $s1, $s1, 1
  b loop
L3:
  bne $t1, $zero, L2
  li $t2, -1
  b exit
L4:
  li $t2, 1
  b exit
L1:
  slt $t3, $t0, $t1
  addi $t3, $t3, -1
  beq $t3, $zero, L2
  li $t2, 1
  b exit
L2:
  move $t2, $zero
exit:
  move $v0, $t2
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
  addi $sp, $sp, -4
  sw $ra, 0($sp)

  # $t0 = compareStrings(str0, str1)
  la $a0, str0
  la $a1, str1
  jal compareStrings
  move $t0, $v0

  # print(log_str0, $t0, log_newline)
  li $v0, 4
  la $a0, log_str0
  syscall
  li $v0, 1
  move $a0, $t0
  syscall
  li $v0, 4
  la $a0, log_newline
  syscall

  # $ra = stack.pop()
  lw $ra, 0($sp)
  addi $sp, $sp, 4

  # return
  jr $ra

