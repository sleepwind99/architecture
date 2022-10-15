#include "Cache.h"

#include <cstring>

// the main memory
BYTE _memory[MEMSIZE];

void printMemory() {
  fprintf(stdout, "========== Memory ==========\n");
  for (size_t addr = 0; addr < MEMSIZE; addr += 4) {
    WORD data = 0;
    for (size_t i = 0; i < 4; i++) {
      data <<= 8;
      data += _memory[addr + (3 - i)];
    }
    if (data != 0)
      fprintf(stdout, "addr = %6zu, data = 0x%08x\n", addr, data);
  }
}

void parseFile(const char *fileName, Cache *cache) {
  FILE *file = fopen(fileName, "r");
  char str[1024];
  while (fgets(str, 1024, file) != NULL) {
    char tok0, tok1[100], tok2[100];
    if (sscanf(str, " %c %s %s", &tok0, tok1, tok2) == 3) {
      if (tok0 == 'I') {
        WORD addr = std::stoul(tok1);
        WORD data = std::stoul(tok2, nullptr, 16);
        for (size_t i = 0; i < 4; i++) {
          _memory[addr + i] = (data % 256);
          data >>= 8;
        }
      } else if (tok0 == 'R') {
        WORD addr = std::stoul(tok1);
        WORD data;
        cache->access(addr, false, &data);
        fprintf(stdout, "INFO: addr = 0x%08x --> Cache --> data = 0x%08x\n", addr, data);
        cache->print();
        printMemory();
        fprintf(stdout, "\n");
      } else if (tok0 == 'W') {
        WORD addr = std::stoul(tok1);
        WORD data = std::stoul(tok2, nullptr, 16);
        cache->access(addr, true, &data);
        fprintf(stdout, "INFO: addr = 0x%08x, data = 0x%08x --> Cache\n", addr, data);
        cache->print();
        printMemory();
        fprintf(stdout, "\n");
      } else {
        fprintf(stderr, "ERROR: Unsupported tok0 '%c'\n", tok0);
        exit(-1);
      }
    }
  }
  fclose(file);
}

int main(int argc, char **argv) {
  if (argc != 6) {
    fprintf(stderr, "Usage: %s numLines numSets lineSize WriteBack<OR>WriteThrough dataFile\n", argv[0]);
    exit(-1);
  }

  const size_t numLines = std::stoul(argv[1]);
  const size_t numSets = std::stoul(argv[2]);
  const size_t lineSize = std::stoul(argv[3]);
  Cache::WritePolicy writePolicy;
  if (strcmp(argv[4], "WriteBack") == 0) {
    writePolicy = Cache::WritePolicy::WRITE_BACK;
  } else if (strcmp(argv[4], "WriteThrough") == 0) {
    writePolicy = Cache::WritePolicy::WRITE_THROUGH;
  } else {
    fprintf(stderr, "ERROR: Unsupported writePolicy '%s'\n", argv[4]);
    exit(-1);
  }
  const char *dataFile = argv[5];

  Cache *cache = new Cache(numLines, numSets, lineSize, writePolicy);
  parseFile(dataFile, cache);
  delete cache;

  return 0;
}

