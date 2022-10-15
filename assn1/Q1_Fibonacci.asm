.data

newline:
  .asciiz "\n"
str0:
  .asciiz "Enter a positive integer: "
str1:
  .asciiz "ERROR: received a negative integer!\n"
str2:
  .asciiz "INFO: fibonacci returned "

.text

################################################################################
# fibonacci
#
# INPUTs
#   $a0: n (<= 32)
#
# OUTPUTs
#   $v0: 0 (if n < 0), 1 (if n >= 0)
#   $v1: F(n) (if n >= 0)
#
tailfbnc:
  addi $sp, $sp, -4
  sw $ra, 0($sp)

  #t1 = 1
  li $t1, 1
  #n = 0 or n = 1
  beq $a0, $zero, L1
  beq $a0, $t1, L2
  
  #a0 = n-1
  addi $a0, $a0, -1
  #a1 = b
  add $t0, $a1, $zero
  add $a1, $a2, $zero
  #a2 = a + b
  add $a2, $a2, $t0
  #tailfbnc(n-1, b, a+b)
  jal tailfbnc
  b exit
#if n = 0
L1:
  move $v1, $a1
  b exit
#if n = 1
L2:
  move $v1, $a2
exit:
  addi $v0, $zero, 1

  lw $ra, 0($sp)
  addi $sp, $sp, 4

  jr $ra

fibonacci:
  addi $sp, $sp, -4
  sw $ra, 0($sp)

  #n < 0
  slt $t0, $a0, $zero
  bnez $t0, xpt

  #a1 = a = 1, a2 = b = 1
  li $a1, 1
  li $a2, 1
  jal tailfbnc
  b end
xpt:
  move $v0, $zero
end:
  lw $ra, 0($sp)
  addi $sp, $sp, 4

  jr $ra
################################################################################

.globl main
main:

  # stack.push($ra)
  # stack.push($s0)
  addi $sp, $sp, -8
  sw $ra, 0($sp)
  sw $s0, 4($sp)

  # print_string str0
  li $v0, 4
  la $a0, str0
  syscall

  # $t0 = read_int
  li $v0, 5
  syscall
  move $t0, $v0

  # $s0 = $ra; fibonacci($t0); $ra = $s0
  move $s0, $ra
  move $a0, $t0
  move $v1, $zero
  jal fibonacci
  move $ra, $s0

  # $t0 = $v0; $t1 = $v1
  move $t0, $v0
  move $t1, $v1

  # if ($t0 == 0) { goto main_failure }
  beq $t0, $zero, main_failure

main_success:

  # print_string str2
  li $v0, 4
  la $a0, str2
  syscall

  # print_int $t1
  li $v0, 1
  move $a0, $t1
  syscall

  # print_string newline
  li $v0, 4
  la $a0, newline
  syscall

  # goto main_return
  b main_return

main_failure:

  # print_string str1
  li $v0, 4
  la $a0, str1
  syscall

main_return:

  # $ra = stack.pop()
  # $s0 = stack.pop()
  sw $ra, 0($sp)
  sw $s0, 4($sp)
  addi $sp, $sp, 8

  # return
  jr $ra

