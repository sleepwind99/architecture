#include "MiniMIPS.h"
#include <cstdlib>
#include <cstdio>
#include <string>

int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s dataFile initialPC numInstrs\n", argv[0]);
    exit(-1);
  }

  const char *dataFile = argv[1];
  const unsigned int initialPC = std::stoul(argv[2]);
  const unsigned int numInstrs = std::stoul(argv[3]);

  MiniMIPS *cpu = new MiniMIPS(dataFile, initialPC);
  for (unsigned int i = 0; i < numInstrs; i++) {
    cpu->execInstr();
  }
  
  // +-- modify as needed!
  // |
  // v
    cpu->printPC();         // print $pc
  cpu->printRegister(4);  // print $a0
  cpu->printRegister(5);  // print $a1
  cpu->printRegister(8);  // print $t0
  cpu->printRegister(9);  // print $t1
  cpu->printRegister(10); // print $t2
  cpu->printRegister(11); // print $t3
  cpu->printRegister(12); // print $t4
  cpu->printRegister(13); // print $t5
  cpu->printRegister(14); // print $t6
  cpu->printRegister(15); // print $t7
  cpu->printRegister(16); // print $t8
  cpu->printRegister(17); // print $t9
  cpu->printMemory(256);  // print memory[259:256]
  cpu->printMemory(260);  // print memory[263:260]
  cpu->printMemory(264);  // print memory[267:264]
  cpu->printMemory(508);  // print memory[259:256]

  /*
  cpu->printPC();         // print $pc
  cpu->printRegister(8);  // print $t0
  cpu->printRegister(9);  // print $t1
  cpu->printRegister(10); // print $t2
  cpu->printRegister(11); // print $t3
  cpu->printMemory(264);  // print memory[267:264]
  */

  // ^
  // |
  // +-- modify as needed!
  
  delete cpu;

  return 0;
}

