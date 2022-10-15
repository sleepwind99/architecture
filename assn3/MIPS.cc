/********************************/
/* Assignment #3: Pipelined CPU */
/********************************/
#include "MIPS.h"

void ALU::advanceCycle() {
  switch(_ctrlInput->to_ulong()){
    //and
    case 0:{
      (*_oResult) = _iInput1->to_ulong() & _iInput2->to_ulong();
      break;
    }
    //or
    case 1:{
      (*_oResult) = _iInput1->to_ulong() | _iInput2->to_ulong();
      break;
    }
    //add
    case 2:{
      (*_oResult) = _iInput1->to_ulong() + _iInput2->to_ulong();
      break;
    }
    //sub
    case 6:{
      (*_oResult) = (int)_iInput1->to_ulong() - (int)_iInput2->to_ulong();
      break;
    }
    //slt
    case 7:{
      (*_oResult) = (int)_iInput1->to_ulong() - (int)_iInput2->to_ulong();
      if((int)_oResult->to_ulong() < 0) (*_oResult) = 1;
      else (*_oResult) = 0;
      break;
    }
  }
  if(_oResult->to_ulong() == 0) (*_oZero) = 1;
  else (*_oZero) = 0;
}

void PipelinedCPU::doInstructionFetch() {
  if(_PCSrc == 1) _PC = _PCSrcInput1;
  else _PC = _PC.to_ulong() + 4;
  _instMem->advanceCycle();
  _latch_IF_ID.PCPlus4 = _PC.to_ulong() + 4;
}

void PipelinedCPU::doInstructionDecode() {
  _latch_ID_EX.PCPlus4 = _latch_IF_ID.PCPlus4;
  unsigned int instr = _latch_IF_ID.inst.to_ulong();
  unsigned int op, rt, rs, rd;
  op = (instr >> 26);
  rs = (instr << 6) >> 27;
  rt = (instr << 11) >> 27;
  rd = (instr << 16) >> 27;
  short imme = (instr << 16) >> 16;
  _latch_ID_EX.signExtdImm = imme;
  _latch_ID_EX.inst_15_11 = rd;
  _latch_ID_EX.inst_20_16 = rt;
  _registersIReadRegister1 = rs;
  _registersIReadRegister2 = rt;
  _latch_ID_EX.ctrl_EX.ALUSrc = 1;
  _latch_ID_EX.ctrl_MEM.Branch = 0;
  _latch_ID_EX.ctrl_MEM.MemRead = 0;
  _latch_ID_EX.ctrl_MEM.MemWrite = 0;
  //R-type
  if(op == 0){
    //write back control
    _latch_ID_EX.ctrl_WB.RegWrite = 1;
    _latch_ID_EX.ctrl_WB.MemToReg = 0;
    //execution control
    _latch_ID_EX.ctrl_EX.ALUSrc = 0;
    _latch_ID_EX.ctrl_EX.RegDst = 1;
    _latch_ID_EX.ctrl_EX.ALUOp[0] = 0;
    _latch_ID_EX.ctrl_EX.ALUOp[1] = 1;
    _registers->advanceCycle();
    return;
  }

  //I-type
  _latch_ID_EX.ctrl_EX.RegDst = 0;
  _latch_ID_EX.ctrl_EX.ALUOp[0] = 0;
  _latch_ID_EX.ctrl_EX.ALUOp[1] = 0;
  _latch_ID_EX.ctrl_WB.RegWrite = 0;
  switch(op){
    //addi
    case 8:{
      //exeception if rt is $zero
      _latch_ID_EX.ctrl_WB.RegWrite = 1;
      break;
    }
    //lw
    case 35:{
      //exeception if rt is $zero
      _latch_ID_EX.ctrl_MEM.MemRead = 1;
      _latch_ID_EX.ctrl_WB.MemToReg = 1;
      _latch_ID_EX.ctrl_WB.RegWrite = 1;
      break;
    }
    //sw
    case 43:{
      _latch_ID_EX.ctrl_MEM.MemWrite = 1;
      break;
    }
    //beq
    case 4:{
      _latch_ID_EX.ctrl_EX.ALUSrc = 0;
      _latch_ID_EX.ctrl_EX.ALUOp[0] = 1;
      _latch_ID_EX.ctrl_MEM.Branch = 1;
      break;
    }
  }
  _registers->advanceCycle();
}

void PipelinedCPU::doExecute() {
  //pass control and data to next latch
  _latch_EX_MEM.ctrl_MEM = _latch_ID_EX.ctrl_MEM;
  _latch_EX_MEM.ctrl_WB = _latch_ID_EX.ctrl_WB;
  _latch_EX_MEM.registersReadData2 = _latch_ID_EX.registersReadData2;
  //branch address calculate
  _latch_EX_MEM.branchTargetAddr = (_latch_ID_EX.signExtdImm.to_ulong() << 2) + _latch_ID_EX.PCPlus4.to_ulong();
  //destination register mux
  if(_latch_ID_EX.ctrl_EX.RegDst == 1) _latch_EX_MEM.writeRegForRegisters = _latch_ID_EX.inst_15_11;
  else _latch_EX_MEM.writeRegForRegisters = _latch_ID_EX.inst_20_16;
  //ALU source mux
  if(_latch_ID_EX.ctrl_EX.ALUSrc == 0) _ALUInput1 = _latch_ID_EX.registersReadData2;
  else _ALUInput1 = _latch_ID_EX.signExtdImm;

  //ALU control 
  if(_latch_ID_EX.ctrl_EX.ALUOp[0] == 0 && _latch_ID_EX.ctrl_EX.ALUOp[1] == 0){
    //add
    _ALUCtrlInput = std::bitset<4>(2);
    _ALU->advanceCycle();
    return;
  }
  if(_latch_ID_EX.ctrl_EX.ALUOp[0] == 1){
    //sub
    _ALUCtrlInput = std::bitset<4>(6);
    _ALU->advanceCycle();
    return;
  }
  unsigned int funcfield = _latch_ID_EX.signExtdImm.to_ulong() & 15;
  switch (funcfield){
    //add
    case 0 : {
      _ALUCtrlInput = std::bitset<4>(2);
      break;
    }
    //sub
    case 2 : {
      _ALUCtrlInput = std::bitset<4>(6);
      break;
    }
    //and
    case 4 : {
      _ALUCtrlInput = std::bitset<4>(0);
      break;
    }
    //or
    case 5 : {
      _ALUCtrlInput = std::bitset<4>(1);
      break;
    }
    //slt
    case 10 : {
      _ALUCtrlInput = std::bitset<4>(7);
      break;
    }
  }
  _ALU->advanceCycle();
}

void PipelinedCPU::doDataMemoryAccess() {
  _PCSrcInput1 = _latch_EX_MEM.branchTargetAddr;
  _latch_MEM_WB.ctrl_WB = _latch_EX_MEM.ctrl_WB;
  _latch_MEM_WB.ALUResult = _latch_EX_MEM.ALUResult;
  _latch_MEM_WB.writeRegForRegisters = _latch_EX_MEM.writeRegForRegisters;
  if(_latch_EX_MEM.ctrl_MEM.ALUZero == 1 && _latch_EX_MEM.ctrl_MEM.Branch == 1) _PCSrc = 1;
  else _PCSrc = 0;
  _dataMem->advanceCycle();
}

void PipelinedCPU::doWriteBack() {
  if(_latch_MEM_WB.ctrl_WB.MemToReg == 1) _registersIWriteData = _latch_MEM_WB.dataMemReadData;
  else _registersIWriteData = _latch_MEM_WB.ALUResult;
  if(_latch_MEM_WB.writeRegForRegisters == 0) _registersIWriteData = 0;
  _registersIWriteRegister = _latch_MEM_WB.writeRegForRegisters;
  if(_latch_MEM_WB.ctrl_WB.RegWrite == 1) _registersCtrlRegWrite = 1;
  else _registersCtrlRegWrite = 0;
}

