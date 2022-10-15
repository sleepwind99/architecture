#ifndef __MINIMIPS_H__
#define __MINIMIPS_H__

#define MEMSIZE 4096

#include <bitset>
#include <cassert>

struct ProgrammerVisibleState {
  unsigned int registers[32]; // general-purpose registers
  unsigned int PC; // program counter
  unsigned char memory[MEMSIZE]; // memory; addr: 0 ~ (MEMSIZE - 1)
};

class MiniMIPS {
public:
  MiniMIPS(const char *dataFile, const unsigned int &initialPC) {
    for (size_t i = 0; i < 32; i++) { m_PVS.registers[i] = 0; }
    for (size_t i = 0; i < MEMSIZE; i++) { m_PVS.memory[i] = 0; }
    parseDataFile(dataFile);
    m_PVS.PC = initialPC;
  }
  void printPC() {
    printf("[%s] PC = %u\n", __func__, m_PVS.PC);
  }
  void printRegister(const unsigned int reg) {
    assert(reg < 32);
    printf("[%s] $%02u = 0x%08x\n", __func__, reg, m_PVS.registers[reg]);
  }
  void printMemory(unsigned int addr, unsigned int size = 4) {
    assert(addr < MEMSIZE);
    assert(size <= MEMSIZE);
    assert(addr + size <= MEMSIZE);
    printf("[%s] memory[%u:%u] = 0x", __func__, addr + size - 1, addr);
    for (int i = addr + size - 1; i >= addr; i--)
      printf("%02x", m_PVS.memory[i]);
    printf("\n");
  }
  ~MiniMIPS() { }
  void execInstr();
private:
  void parseDataFile(const char *dataFile) {
    assert(dataFile != nullptr);
    FILE *file = fopen(dataFile, "r");
    assert(file != NULL);
    char tok0[100], tok1[100];
    while (fscanf(file, " %s %s", tok0, tok1) != EOF) {
      if (tok0[0] == '$') {
        // register
        const unsigned int reg = std::stoul(&tok0[1]);
        const unsigned int val = std::stoul(tok1, nullptr, 16);
        printf("[%s] $%02u = 0x%08x\n", __func__, reg, val);
        m_PVS.registers[reg] = val;
      } else {
        // memory
        const unsigned int addr = std::stoul(tok0);
        unsigned int val = std::stoul(tok1, nullptr, 16);
        printf("[%s] memory[%u:%u] = 0x%08x\n", __func__, addr + 3, addr, val);
        for (unsigned int i = 0; i < 4; i++) {
          m_PVS.memory[addr + i] = val % 256;
          val /= 256;
        }
      }
    }
    fclose(file);
  }
  ProgrammerVisibleState m_PVS;
};

#endif

