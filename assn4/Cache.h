#ifndef __CACHE_H__
#define __CACHE_H__

#include <bitset>
#include <string>
#include <iomanip>
#include <cstdio>
#include <cassert>
#include <cstdint>
#include <cstdlib>

typedef std::uint8_t BYTE;
typedef std::uint32_t WORD;

// cache
class Cache {
public:
  enum WritePolicy { WRITE_THROUGH, WRITE_BACK };
  Cache(const size_t numLines, const size_t numSets, const size_t lineSize,
    const WritePolicy writePolicy) : _numLines(numLines), _numSets(numSets),
    _lineSize(lineSize), _writePolicy(writePolicy) {
    assert(_numLines > 0);
    assert(_numSets > 0);
    assert(_numLines % _numSets == 0);

    // the line size is a multiple of word
    assert(lineSize % 4 == 0);

    _lines = new Line*[_numSets];
    for (size_t i = 0; i < _numSets; i++) {
      _lines[i] = new Line[_numLines / _numSets];
      for (size_t j = 0; j < _numLines / _numSets; j++) {
        _lines[i][j].valid = false;
        _lines[i][j].tag = 0;
        _lines[i][j].data = new BYTE[_lineSize];
        for (size_t k = 0; k < _lineSize; k++)
          _lines[i][j].data[k] = 0;
        _lines[i][j].lruCounter = 0;
        if (_writePolicy == WRITE_BACK)
          _lines[i][j].dirty = false;
      }
    }
  }
  ~Cache() {
    for (size_t i = 0; i < _numSets; i++) {
      for (size_t j = 0; j < _numLines / _numSets; j++)
        delete[] _lines[i][j].data;
      delete[] _lines[i];
    }
    delete[] _lines;
  }
private:
  size_t _numLines, _numSets, _lineSize;
  WritePolicy _writePolicy;
private:
  typedef struct {
    bool valid;
    WORD tag;
    BYTE *data;
    size_t lruCounter; // 0 upon an access, += 1 for no access.
    bool dirty; // for write-back; true if dirty, false if clean.
  } Line;
  Line **_lines = nullptr;
public:
  void print() {
    fprintf(stdout, "========== Cache Status ==========\n");
    fprintf(stdout, "_numLines = %zu, _numSets = %zu, _lineSize = %zu\n", _numLines, _numSets, _lineSize);
    for (size_t i = 0; i < _numSets; i++) {
      fprintf(stdout, "Set #%zu:\n", i);
      for (size_t j = 0; j < _numLines / _numSets; j++) {
        fprintf(stdout, "  Line #%zu: valid = %c", j, _lines[i][j].valid ? 'T' : 'F');
        fprintf(stdout, ", tag = 0x%x", _lines[i][j].tag);
        fprintf(stdout, ", data = 0x");
        for (size_t k = 0; k < _lineSize; k++)
          fprintf(stdout, "%02x", _lines[i][j].data[(_lineSize - 1) - k]);
        fprintf(stdout, ", lruCounter = %zu", _lines[i][j].lruCounter);
        if (_writePolicy == WRITE_BACK)
          fprintf(stdout, ", dirty = %c", _lines[i][j].dirty ? 'T' : 'F');
        fprintf(stdout, "\n");
      }
    }
  }
public:
  void access(const WORD addr, const bool isWrite, WORD *data);
};

#endif

