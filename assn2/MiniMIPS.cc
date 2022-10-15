#include "MiniMIPS.h"
#include <cstdio>

#define OPDIV (256*256*256*4)
#define RSDIV (256*256*32)
#define RTDIV (256*256)
#define RDDIV (256*8)
#define SHDIV  128

void MiniMIPS::execInstr() {
  unsigned int instr = 0;

  //fetch
  for(unsigned int i = 0; i < 4; i++) {
    unsigned int k = 1;
    for(unsigned int j = 0; j < i; j++) k *= 256;
    instr += m_PVS.memory[m_PVS.PC + i] * k;
  }
  
  printf("[%s] PC = %u, instr = 0x%08x\n", __func__, m_PVS.PC, instr);

  //decode and execution
  unsigned int op, rs, rt, rd, shamt, funct;
  op = instr / OPDIV;
  instr -= op * OPDIV;

  //J-type
  if(op == 2){
    //J
    unsigned int immediate = (instr * 4) + 4;
    unsigned int addr = m_PVS.PC - (m_PVS.PC / (OPDIV * 4));
    m_PVS.PC -= addr;
    m_PVS.PC += immediate;
    return;
  }

  //rs, rt both using I-type, R-type instructions
  rs = instr / RSDIV;
  instr -= rs * RSDIV;
  rt = instr / RTDIV;
  instr -= rt * RTDIV;

  //R-type
  if(op == 0){
    //rest of rd, shamt, funct using R-type
    rd = instr / RDDIV;
    instr -= rd * RDDIV;
    shamt = instr / SHDIV;
    instr -= shamt * SHDIV;
    funct = instr;
    //exeception if rd is $zero
    if(rd == 0){
      m_PVS.PC += 4;
      return;
    }
    //operations divide by funct value
    switch (funct){
      //add
      case 32:{
        m_PVS.registers[rd] = (int)m_PVS.registers[rs] + (int)m_PVS.registers[rt];
        break;
      }
      //sub
      case 34:{
        m_PVS.registers[rd] = (int)m_PVS.registers[rs] - (int)m_PVS.registers[rt];
        break;
      }
      //and
      case 36:{
        m_PVS.registers[rd] = m_PVS.registers[rs] & m_PVS.registers[rt];
        break;
      }
      //or
      case 37:{
        m_PVS.registers[rd] = m_PVS.registers[rs] | m_PVS.registers[rt];
        break;
      }
      //slt
      case 42:{
        if((int)m_PVS.registers[rs] < (int)m_PVS.registers[rt]) m_PVS.registers[rd] = 1;
        else m_PVS.registers[rd] = 0;
        break;
      }
    }
  }
  //I-type
  else{
    //operations divide by op value
    switch(op){
      //addi
      case 8:{
        //exeception if rt is $zero
        if(rt == 0) break;
        short immediate = (short)instr;
        m_PVS.registers[rt] = (int)m_PVS.registers[rs] + immediate;
        break;
      }
      //lw
      case 35:{
        //exeception if rt is $zero
        if(rt == 0) break;
        short immediate = (short)instr;
        unsigned int data = 0;
        for(unsigned int i = 0; i < 4; i++) {
          unsigned int k = 1;
          for(unsigned int j = 0; j < i; j++) k *= 256;
          data += m_PVS.memory[m_PVS.registers[rs] + immediate + i] * k;
        }
        m_PVS.registers[rt] = data;
        break;
      }
      //sw
      case 43:{
        short immediate = (short)instr;
        unsigned int data = m_PVS.registers[rt];
        for (unsigned int i = 0; i < 4; i++) {
          m_PVS.memory[m_PVS.registers[rs] + immediate + i] =  data % 256;
          data /= 256;
        }
        break;
      }
      //beq
      case 4:{
        short immediate = (short)instr;
        if(m_PVS.registers[rs] == m_PVS.registers[rt]) m_PVS.PC += immediate*4;
        break;
      }
    }
  }
  //PC + 4
  m_PVS.PC += 4;
}