#ifndef CTripleBufferManager_H
#define CTripleBufferManager_H

#include <memory>
#include <mutex>
#include <vector>
#include <string.h>

#define BUFFER_CONTAINER_SIZE 3

// very basic and inacurate buffer class only for demonstration
struct CBuffer {
  virtual ~CBuffer()
  {
    _buffer.reset();
  }

  void Create(int par_nColSize, int par_nRowSize, int stride = 0) { 
    _buffer_size = (par_nColSize + stride) * par_nRowSize;
    _buffer.reset(new unsigned char[_buffer_size]);
  }

  void setAllBytes(int initial) {
    memset(_buffer.get(), initial, _buffer_size);
  }

  void Destroy() { 
    _buffer.reset();
  }

  void CopyFrom(const CBuffer& other) { 
    _buffer_size = other._buffer_size;
    _buffer.reset(new unsigned char[other._buffer_size]);
    memcpy(_buffer.get(), other._buffer.get(), _buffer_size);
  }

  unsigned char& operator[](size_t _Idx) const
  {	// return reference to object
    return _buffer[_Idx];
  }

private:
  std::unique_ptr<unsigned char[]> _buffer;
  size_t _buffer_size;
};

enum teContainerState {
  cs_LockedForReading,
  cs_LockedForWriting,
  cs_AvailableForNextWriting
};

// pimpl buffer container state manager
class CTripleBuffContainer;

class CTripleBufferManager {
public :
  CTripleBufferManager(int par_nColumnSize, int par_nRowSize, int par_nDefaultValue = 0);
  ~CTripleBufferManager();
  CTripleBufferManager() = delete;

  CBuffer * GetNewReadingBuffer();
  CBuffer * GetNewWritingBuffer();

private :
  
  int GetIndexForFirstAvailableForReading();
  int GetIndexForFirstAvailableForWriting();

  // for testing only
  friend void printvalues(const CTripleBufferManager&);
  void print_sync(std::ostream& out) const;    
protected :
  std::vector<CTripleBuffContainer> buffTripleContainer;
    
  std::mutex itsCMutex;
  int nAvailableBuffIndex;
  int nReadingBuffIndex;
  int nWritingBuffIndex;
};

#endif
