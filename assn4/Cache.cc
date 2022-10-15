#include "Cache.h"
// '_memory' defined in testCache.cc
extern BYTE _memory[MEMSIZE];

/*************************************************************************************/
/* Assignment #4: Caches                                                             */
/*                                                                                   */
/* addr: target memory address                                                       */
/* isWrite: true if write, false if read                                             */
/* data: data to be written to addr if isWrite, data read from the cache if !isWrite */
/*************************************************************************************/
void Cache::access(const WORD addr, const bool isWrite, WORD *data) {
  assert(addr % 4 == 0);
  assert(data != nullptr);
  size_t tagaddr = addr / (_numSets * _lineSize);
  size_t address = addr % (_numSets * _lineSize);
  size_t ti, offset, delidx, maxcount = 0;
  ti = address / _lineSize;
  offset = address % _lineSize;
  bool ishit = false;
  //////////////////////////write mode/////////////////////////////
  if(isWrite){
    for (size_t j = 0; j < _numLines / _numSets; j++) {
      //if hit
      if(_lines[ti][j].valid && _lines[ti][j].tag == tagaddr){
        _lines[ti][j].lruCounter = 0;
        for(size_t i = 0; i < 4; i++)
          _lines[ti][j].data[offset + i] = ((*data) >> (i*8)) & 255;
        //WRITE_THROUGH
        if(_writePolicy == WRITE_THROUGH){
          size_t memaddr = _lines[ti][j].tag * (_numSets * _lineSize);
          memaddr += ti * _lineSize;
          for(size_t i = 0; i < _lineSize; i++)
            _memory[memaddr + i] = _lines[ti][j].data[i];
        }//WRITE_BACK
        else _lines[ti][j].dirty = true;
        ishit = true;
        continue;
      }
      if(_lines[ti][j].valid) _lines[ti][j].lruCounter++;
    }
    if(ishit) return;
    //if not hit
    //searching biggest lruCounter or invalid cache index
    for (size_t j = 0; j < _numLines / _numSets; j++) {
      if(!_lines[ti][j].valid){
        delidx = j;
        break;
      }
      if(maxcount < _lines[ti][j].lruCounter){
        delidx = j;
        maxcount = _lines[ti][j].lruCounter;
      }
    }
    size_t memaddr = _lines[ti][delidx].tag * (_numSets * _lineSize);
    memaddr += ti * _lineSize;
    //WRITE_BACK
    if(_writePolicy == WRITE_BACK){
      if(_lines[ti][delidx].dirty){
        for(size_t i = 0; i < _lineSize; i++) 
          _memory[memaddr + i] = _lines[ti][delidx].data[i];
      }
      _lines[ti][delidx].dirty = true;
      
    }
    for(size_t i = 0; i < 4; i++)
      _lines[ti][delidx].data[offset + i] = ((*data) >> (i*8)) & 255;
    //WRITE_THROUGH
    if(_writePolicy == WRITE_THROUGH){
      for(size_t i = 0; i < _lineSize; i++)
        _memory[addr + i] = _lines[ti][delidx].data[i];
    }
    _lines[ti][delidx].tag = tagaddr;
    _lines[ti][delidx].lruCounter = 0;
    _lines[ti][delidx].valid = true;
  }////////////////////////////read mode///////////////////////////////
  else{
    (*data) = 0;
    for (size_t j = 0; j < _numLines / _numSets; j++) {
      //if hit
      if(_lines[ti][j].valid && _lines[ti][j].tag == tagaddr){
        _lines[ti][j].lruCounter = 0;
        for(size_t i = 0; i < 4; i++) 
          (*data) += _lines[ti][j].data[offset + i] << (i*8);
        ishit = true;
        continue;
      }
      if(_lines[ti][j].valid) _lines[ti][j].lruCounter++;
    }
    if(ishit) return;
    //if not hit
    //searching biggest lruCounter or invalid index
    for (size_t j = 0; j < _numLines / _numSets; j++) {
      if(!_lines[ti][j].valid){
        delidx = j;
        break;
      }
      if(maxcount < _lines[ti][j].lruCounter){
        delidx = j;
        maxcount = _lines[ti][j].lruCounter;
      }
    }
    //if WRITE_BACK
    if(_writePolicy == WRITE_BACK && _lines[ti][delidx].dirty){
      size_t memaddr = _lines[ti][delidx].tag * (_numSets * _lineSize);
      memaddr += ti * _lineSize;
      for(size_t i = 0; i < _lineSize; i++)
        _memory[memaddr + i] = _lines[ti][delidx].data[i];
      _lines[ti][delidx].dirty = false;
    }
    size_t memaddr = addr - (addr % _lineSize);
    for(size_t i = 0; i < _lineSize; i++)
      _lines[ti][delidx].data[i] = _memory[memaddr + i];
    for(size_t i = 0; i < 4; i++)
      (*data) += _lines[ti][delidx].data[offset + i] << (i * 8);
    _lines[ti][delidx].tag = tagaddr;
    _lines[ti][delidx].lruCounter = 0;
    _lines[ti][delidx].valid = true;
  }
}

