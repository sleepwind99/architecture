#ifndef __MIPS_H__
#define __MIPS_H__

#include <bitset>
#include <iostream>
#include <cassert>
#include <cstdlib>

// data sizes
typedef std::bitset< 1> BIT;
typedef std::bitset< 8> BYTE;
typedef std::bitset<32> WORD;

class DigitalCircuit {
public:
  virtual void advanceCycle() = 0;
};

class Memory : public DigitalCircuit {
public:
  Memory(
    const size_t size,
    const WORD *iAddress,
    const WORD *iWriteData,
    WORD *oReadData,
    const BIT *ctrlMemRead = nullptr,
    const BIT *ctrlMemWrite = nullptr
  ) {
    assert(size > 0 && (size % 4 == 0));
    _size = size;
    _buffer = new BYTE[_size];
    _iAddress = iAddress;
    _iWriteData = iWriteData;
    _oReadData = oReadData;
    _ctrlMemRead = ctrlMemRead;
    _ctrlMemWrite = ctrlMemWrite;
  }
  ~Memory() {
    delete[] _buffer;
  }
private:
  size_t _size;
  BYTE *_buffer;
  const WORD *_iAddress, *_iWriteData;
  WORD *_oReadData;
  const BIT *_ctrlMemRead, *_ctrlMemWrite;
public:
  void advanceCycle() {
    if (_ctrlMemRead == nullptr && _ctrlMemWrite == nullptr) {
      // instruction memory
      assert(_iAddress->to_ulong() + 4 <= _size);
      for (size_t i = 0; i < 4; i++) {
        for (size_t j = 0; j < 8; j++) {
          _oReadData->set(i * 8 + j, _buffer[_iAddress->to_ulong() + i][j]);
        }
      }
    } else {
      // data memory
      assert(_ctrlMemRead != nullptr && _ctrlMemWrite != nullptr);
      assert(!(_ctrlMemRead->all() && _ctrlMemWrite->all()));
      if (_ctrlMemRead->all()) {
	    	assert(_iAddress->to_ulong() + 4 <= _size);
        for (size_t i = 0; i < 4; i++) {
          for (size_t j = 0; j < 8; j++) {
          	_oReadData->set(i * 8 + j, _buffer[_iAddress->to_ulong() + i][j]);
          }
        }
      }
      if (_ctrlMemWrite->all()) {
	    	assert(_iAddress->to_ulong() + 4 <= _size);
        for (size_t i = 0; i < 4; i++) {
          for (size_t j = 0; j < 8; j++) {
            _buffer[_iAddress->to_ulong() + i][j] = (*_iWriteData)[i * 8 + j];
          }
        }
      }
    }
  }
public:
	void initialize(const size_t addr, const WORD data) {
		assert(addr + 4 <= _size);
		for (size_t i = 0; i < 4; i++) {
			for (size_t j = 0; j < 8; j++) {
				_buffer[addr + i][j] = data[i * 8 + j];
			}
		}
	}
  void print() {
    for (size_t i = 0; i < _size; i += 4) {
      WORD data;
      for (size_t j = 0; j < 4; j++) {
        for (size_t k = 0; k < 8; k++) {
          data[j * 8 + k] = _buffer[i + j][k];
        }
      }
      if (data.any()) {
        printf("  %zu = 0x%08lx\n", i, data.to_ulong());
      }
    }
  }
};

class RegisterFile : public DigitalCircuit {
public:
  RegisterFile(
    const std::bitset<5> *iReadRegister1,
    const std::bitset<5> *iReadRegister2,
    const std::bitset<5> *iWriteRegister,
    const WORD *iWriteData,
    WORD *oReadData1,
    WORD *oReadData2,
    const BIT *ctrlRegWrite
  ) {
    _registers = new WORD[32];
    for (size_t i = 0; i < 32; i++) { _registers[i] = 0; }
    _iReadRegister1 = iReadRegister1;
    _iReadRegister2 = iReadRegister2;
    _iWriteRegister = iWriteRegister;
    _iWriteData = iWriteData;
    _oReadData1 = oReadData1;
    _oReadData2 = oReadData2;
    _ctrlRegWrite = ctrlRegWrite;
  }
  ~RegisterFile() {
    delete[] _registers;
  }
private:
  WORD *_registers;
  const std::bitset<5> *_iReadRegister1, *_iReadRegister2, *_iWriteRegister;
  const WORD *_iWriteData;
  WORD *_oReadData1, *_oReadData2;
  const BIT *_ctrlRegWrite;
public:
  void advanceCycle() {
    // write in the first half of a cycle
    if (_ctrlRegWrite->all() && _iWriteRegister->to_ulong() != 0) {
      _registers[_iWriteRegister->to_ulong()] = (*_iWriteData);
    }
    // reads in the second half of the cycle
    (*_oReadData1) = _registers[_iReadRegister1->to_ulong()];
    (*_oReadData2) = _registers[_iReadRegister2->to_ulong()];
  }
public:
	void initialize(const size_t reg, const WORD value) {
		assert(reg < 32);
		_registers[reg] = value;
	}
  void print() {
    printf("Registers\n");
    for (size_t i = 0; i < 32; i++) {
      if (_registers[i].any()) {
        printf("  $%zu = 0x%08lx\n", i, _registers[i].to_ulong());
      }
    }
  }
};

class ALU : public DigitalCircuit {
public:
  ALU(
    const WORD *iInput1,
    const WORD *iInput2,
    BIT *oZero,
    WORD *oResult,
    const std::bitset<4> *ctrlInput
  ) {
    _iInput1 = iInput1;
    _iInput2 = iInput2;
    _oZero = oZero;
    _oResult = oResult;
    _ctrlInput = ctrlInput;
  }
private:
  const WORD *_iInput1, *_iInput2;
  BIT *_oZero;
  WORD *_oResult;
  const std::bitset<4> *_ctrlInput;
public:
  void advanceCycle();
};

class PipelinedCPU : public DigitalCircuit {
public:
  PipelinedCPU(
    const char *dataFile,
    const WORD initialPC = 0,
    const size_t instMemSize = 8192,
    const size_t dataMemSize = 8192
  ) {
    _instMem = new Memory(instMemSize, &_PC, nullptr, &_latch_IF_ID.inst);
    _registers = new RegisterFile(&_registersIReadRegister1,
      &_registersIReadRegister2, &_registersIWriteRegister,
      &_registersIWriteData, &_latch_ID_EX.registersReadData1,
      &_latch_ID_EX.registersReadData2, &_registersCtrlRegWrite);
    _ALU = new ALU(&_latch_ID_EX.registersReadData1, &_ALUInput1,
      &_latch_EX_MEM.ctrl_MEM.ALUZero, &_latch_EX_MEM.ALUResult,
      &_ALUCtrlInput);
    _dataMem = new Memory(dataMemSize, &_latch_EX_MEM.ALUResult,
      &_latch_EX_MEM.registersReadData2, &_latch_MEM_WB.dataMemReadData,
      &_latch_EX_MEM.ctrl_MEM.MemRead, &_latch_EX_MEM.ctrl_MEM.MemWrite);

    _PC = initialPC;
    this->initialize(dataFile);
  }
  void advanceCycle() final {
    doWriteBack();
    doDataMemoryAccess();
    doExecute();
    doInstructionDecode();
    doInstructionFetch();
  }
private:
  // datapath
  WORD _PC = 0; // PC register
  Memory *_instMem = nullptr; // instruction memory
  RegisterFile *_registers = nullptr; // registers
  ALU *_ALU = nullptr; // ALU
  Memory *_dataMem = nullptr; // data memory 
private:
  // the five pipeline stages
  void doInstructionFetch();
  void doInstructionDecode();
  void doExecute();
  void doDataMemoryAccess();
  void doWriteBack();
private:
  // control signals
  typedef struct {
    BIT RegDst;
    BIT ALUSrc;
    std::bitset<2> ALUOp;
  } ctrl_EX_t;
  typedef struct {
    BIT Branch;
    BIT MemRead;
    BIT MemWrite;
    BIT ALUZero;
  } ctrl_MEM_t;
  typedef struct {
    BIT RegWrite;
    BIT MemToReg;
  } ctrl_WB_t;
private:
  // latches between the five pipeline stages
  struct {
    WORD PCPlus4; // PC + 4
    WORD inst; // the instruction from the instruction memory
  } _latch_IF_ID = {};
  struct {
    WORD PCPlus4; // PC + 4
    WORD registersReadData1; // 'ReadData1' from the registers
    WORD registersReadData2; // 'ReadData2' from the registers
    WORD signExtdImm; // the 32-bit sign-extended immediate
    std::bitset<5> inst_20_16; // instruction[20:16]
    std::bitset<5> inst_15_11; // instruction[15:11]
    ctrl_EX_t ctrl_EX;
    ctrl_MEM_t ctrl_MEM;
    ctrl_WB_t ctrl_WB;
  } _latch_ID_EX = {};
  struct {
    WORD branchTargetAddr; // the target address for branches
    WORD ALUResult; // 'Result' from the ALU
    WORD registersReadData2; // 'ReadData2' from the registers
    std::bitset<5> writeRegForRegisters; // 'WriteRegister' for the registers
    ctrl_MEM_t ctrl_MEM;
    ctrl_WB_t ctrl_WB;
  } _latch_EX_MEM = {};
  struct {
    WORD ALUResult; // 'Result' from the ALU
    WORD dataMemReadData; // 'ReadData' from the data memory
    std::bitset<5> writeRegForRegisters; // 'WriteRegister' for the registers
    ctrl_WB_t ctrl_WB;
  } _latch_MEM_WB = {};
private:
  // wires for the PC register
  BIT _PCSrc = 0;
  WORD _PCSrcInput1 = 0; // 2nd input (i.e., 1) to the IF stage's multiplexer
  // wires for the registers
  std::bitset<5> _registersIReadRegister1 = 0;
  std::bitset<5> _registersIReadRegister2 = 0;
  std::bitset<5> _registersIWriteRegister = 0;
  WORD _registersIWriteData = 0;
  BIT _registersCtrlRegWrite = 0;
  // wires for the ALU
  WORD _ALUInput1 = 0; // 2nd input to the ALU
  std::bitset<4> _ALUCtrlInput = 0; // control input to the ALU
public:
  void initialize(const char *filename) {
    FILE *file = fopen(filename, "r");
    char str[1024];
    while (fgets(str, 1024, file) != NULL) {
      char tok0[100], tok1[100];
      if (sscanf(str, " %s %s", tok0, tok1) != EOF) {
  			switch (tok0[0]) {
  				case '$': // register
  					_registers->initialize(std::stoul(&tok0[1]), std::stoul(tok1, nullptr, 16));
  					break;
  				case 'i': // instruction memory
  					_instMem->initialize(std::stoul(&tok0[1]), std::stoul(tok1, nullptr, 16));
  					break;
  				case 'd': // data memory
  					_dataMem->initialize(std::stoul(&tok0[1]), std::stoul(tok1, nullptr, 16));
  					break;
  			}
      }
    }
    fclose(file);
  }
  void print() {
    printf("_PC = %lu\n", _PC.to_ulong());
    this->printInstMem();
    this->printDataMem();
    this->printRegisters();
    printf("Latches\n");
    printf("  IF-ID\n");
    printf("    PCPlus4 = %lu\n", _latch_IF_ID.PCPlus4.to_ulong());
    printf("    inst = 0x%08lx\n", _latch_IF_ID.inst.to_ulong());
    printf("  ID-EX\n");
    printf("    PCPlus4 = %lu\n", _latch_ID_EX.PCPlus4.to_ulong());
    printf("    registersReadData1 = 0x%08lx\n", _latch_ID_EX.registersReadData1.to_ulong());
    printf("    registersReadData2 = 0x%08lx\n", _latch_ID_EX.registersReadData2.to_ulong());
    printf("    signExtdImm = 0x%08lx\n", _latch_ID_EX.signExtdImm.to_ulong());
    printf("    inst_20_16 = 0b%s\n", _latch_ID_EX.inst_20_16.to_string().c_str());
    printf("    inst_15_11 = 0b%s\n", _latch_ID_EX.inst_15_11.to_string().c_str());
    printf("    ctrl_EX\n");
    printf("      RegDst = %lu\n", _latch_ID_EX.ctrl_EX.RegDst.to_ulong());
    printf("      ALUSrc = %lu\n", _latch_ID_EX.ctrl_EX.ALUSrc.to_ulong());
    printf("      ALUOp  = %lu\n", _latch_ID_EX.ctrl_EX.ALUOp.to_ulong());
    printf("    ctrl_MEM\n");
    printf("      Branch   = %lu\n", _latch_ID_EX.ctrl_MEM.Branch.to_ulong());
    printf("      MemRead  = %lu\n", _latch_ID_EX.ctrl_MEM.MemRead.to_ulong());
    printf("      MemWrite = %lu\n", _latch_ID_EX.ctrl_MEM.MemWrite.to_ulong());
    printf("      ALUZero  = %lu\n", _latch_ID_EX.ctrl_MEM.ALUZero.to_ulong());
    printf("    ctrl_WB\n");
    printf("      RegWrite = %lu\n", _latch_ID_EX.ctrl_WB.RegWrite.to_ulong());
    printf("      MemToReg = %lu\n", _latch_ID_EX.ctrl_WB.MemToReg.to_ulong());
		printf("  EX-MEM\n");
		printf("    branchTargetAddr = %lu\n", _latch_EX_MEM.branchTargetAddr.to_ulong());
		printf("    ALUResult = 0x%08lx\n", _latch_EX_MEM.ALUResult.to_ulong());
		printf("    registersReadData2 = 0x%08lx\n", _latch_EX_MEM.registersReadData2.to_ulong());
    printf("    writeRegForRegisters = 0b%s\n", _latch_EX_MEM.writeRegForRegisters.to_string().c_str());
    printf("    ctrl_MEM\n");
    printf("      Branch   = %lu\n", _latch_EX_MEM.ctrl_MEM.Branch.to_ulong());
    printf("      MemRead  = %lu\n", _latch_EX_MEM.ctrl_MEM.MemRead.to_ulong());
    printf("      MemWrite = %lu\n", _latch_EX_MEM.ctrl_MEM.MemWrite.to_ulong());
    printf("      ALUZero  = %lu\n", _latch_EX_MEM.ctrl_MEM.ALUZero.to_ulong());
    printf("    ctrl_WB\n");
    printf("      RegWrite = %lu\n", _latch_EX_MEM.ctrl_WB.RegWrite.to_ulong());
    printf("      MemToReg = %lu\n", _latch_EX_MEM.ctrl_WB.MemToReg.to_ulong());
 		printf("  MEM-WB\n");
		printf("    dataMemReadData = 0x%08lx\n", _latch_MEM_WB.dataMemReadData.to_ulong());
		printf("    ALUResult = 0x%08lx\n", _latch_MEM_WB.ALUResult.to_ulong());
    printf("    writeRegForRegisters = 0b%s\n", _latch_MEM_WB.writeRegForRegisters.to_string().c_str());
    printf("    ctrl_WB\n");
    printf("      RegWrite = %lu\n", _latch_MEM_WB.ctrl_WB.RegWrite.to_ulong());
    printf("      MemToReg = %lu\n", _latch_MEM_WB.ctrl_WB.MemToReg.to_ulong());
  }
  void printRegisters() {
    _registers->print();
  }
  void printInstMem() {
    printf("Instruction Memory\n");
    _instMem->print();
  }
  void printDataMem() {
    printf("Data Memory\n");
    _dataMem->print();
  }
};

#endif

