#include "MIPS.h"

#include <cstdio>
#include <cstdlib>

int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s dataFile initialPC numCycles\n", argv[0]);
    exit(-1);
  }

  const char *dataFile = argv[1];
  const WORD initialPC(std::stoul(argv[2]));
  const size_t numCycles = std::stoul(argv[3]);

  PipelinedCPU *cpu = new PipelinedCPU(dataFile, initialPC);
  printf("========== Cycle %u ==========\n", 0);
  cpu->print();
  for (size_t i = 1; i <= numCycles; i++) {
    cpu->advanceCycle();
    printf("========== Cycle %lu ==========\n", i);
    cpu->print();
  }
  delete cpu;

  return 0;
}

