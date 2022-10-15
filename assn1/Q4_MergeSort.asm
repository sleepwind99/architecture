.data

info0:
  .asciiz "The string to be sorted is: "
info1:
  .asciiz "The sorted string is: "
newline:
  .asciiz "\n"

str_buf:
  .asciiz "abcdefgHIJKLMNOP"
str_len:
  .word 16

.text

################################################################################
# merge
#
# INPUTs
#   $a0: the starting memory address of a string (i.e., str)
#   $a1: the first index of the range of the string to be merged (i.e., l)
#   $a2: the middle index of the range of the string to be merged (i.e., m)
#   $a3: the last index of the range of the string to be merged (i.e., r)
#
merge:
  # stack.push($ra) <-- modify this part depending w.r.t. your implementation
  addi $sp, $sp, -4
  sw $ra, 0($sp)
  
while:
  #(l < m && m < r)
  slt $t0, $a1, $a2
  slt $t1, $a2, $a3
  bne $t0, $t1, exit
  beqz $t0, exit

  #t2 = str[l]
  add $t0, $a1, $a0
  lb $t2, 0($t0)

  #t3 = str[m]
  add $t0, $a2, $a0
  lb $t3, 0($t0)

  #str[m] < str[l]
  slt $t0, $t3, $t2
  addi $t0, $t0, -1
  bnez $t0, L1

  #t4 = i
  add $t4, $a2, $zero
for:
  #i > l
  slt $t0, $a1, $t4
  addi $t0, $t0, -1
  bnez $t0, L2
  #for body
  addi $t4, $t4, -1
  add $t0, $a0, $t4
  lb $t1, 0($t0)
  addi $t4, $t4, 1
  add $t0, $a0, $t4
  sb $t1, 0($t0)
  addi $t4, $t4, -1
  b for
L2:
  #tmp = str[m] = t3
  add $t0, $a1, $a0
  sb $t3, 0($t0) 
  addi $a2, $a2, 1
L1:
  addi $a1, $a1, 1
  b while
#not while
exit:
  # $ra = stack.pop() <-- modify this part depending w.r.t. your implementation
  lw $ra, 0($sp)
  addi $sp, $sp, 4

  jr $ra
################################################################################

################################################################################
# mergeSort
#
# INPUTs
#   $a0: the starting memory address of a string (i.e., str)
#   $a1: the first index of the range of the string to be sorted (i.e., l)
#   $a2: the last index of the range of the string to be sorted (i.e., r)
#
mergeSort:
  # stack.push($ra) <-- modify this part depending w.r.t. your implementation
  addi $sp, $sp, -16
  sw $ra, 12($sp)
  sw $s0, 8($sp)
  sw $s1, 4($sp)
  sw $s2, 0($sp)
  #s0 = l, s1 = r
  move $s0, $a1
  move $s1, $a2
  #l < r-1
  addi $t0, $s1, -1
  slt $t1, $s0, $t0
  addi $t1, $t1, -1
  bnez $t1, end

  #s2 = m
  add $t0, $s0, $s1
  addi $t0, $t0, 1
  li $t1, 2
  div $t0, $t1
  mflo $s2

  add $a1, $s0, $zero
  add $a2, $s2, $zero
  jal mergeSort

  add $a1, $s2, $zero
  add $a2, $s1, $zero
  jal mergeSort

  add $a1, $s0, $zero
  add $a2, $s2, $zero
  add $a3, $s1, $zero
  jal merge

end:
  lw $ra, 12($sp)
  lw $s0, 8($sp)
  lw $s1, 4($sp)
  lw $s2, 0($sp)
  addi $sp, $sp, 16

  jr $ra
################################################################################

.globl main
main:

  # stack.push($ra)
  addi $sp, $sp, -4
  sw $ra, 0($sp)

  # print_string(info0)
  li $v0, 4
  la $a0, info0
  syscall
  # print_string(str_buf)
  li $v0, 4
  la $a0, str_buf
  syscall
  # print_string(newline)
  li $v0, 4
  la $a0, newline
  syscall

  # mergeSort(str_buf, 0, str_len)
  la $a0, str_buf
  li $a1, 0
  lw $a2, str_len
  jal mergeSort

  # print_string(info1)
  li $v0, 4
  la $a0, info1
  syscall
  # print_string(str_buf)
  li $v0, 4
  la $a0, str_buf
  syscall
  # print_string(newline)
  li $v0, 4
  la $a0, newline
  syscall

  # stack.pop($ra)
  lw $ra, 0($sp)
  addi $sp, $sp, 4

  # return
  jr $ra

