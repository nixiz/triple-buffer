#include "CTripleBufferManager.h"
#include <cassert>
#include <sstream>
#include <ostream>

#ifndef _ASSERT
#define _ASSERT assert
#endif

enum teContainerState {
  cs_LockedForReading,
  cs_LockedForWriting,
  cs_AvailableForNextWriting
};

class CTripleBuffContainer {
public:
  CTripleBuffContainer();
  ~CTripleBuffContainer();

  void Create(int par_nColSize, int par_nRowSize, int par_nDefaultValue = 0);
  void Destroy();
  CBuffer * GetBuffer() const;
  teContainerState GetState() const;
  bool HasDataWrittenAndReadyToRead();
  void SetState(const teContainerState& par_eState);
protected:
  bool bCreated;
  mutable CBuffer buff;
  teContainerState eState;
  teContainerState eStatePre;
};

void CTripleBufferManager::print_sync(std::ostream& out) const
{
  for (int i = 0; i < BUFFER_CONTAINER_SIZE; i++) {
    out
      << "slot[" << i << "] details: " << std::endl
      << "state: ";
    switch (buffTripleContainer[i].GetState())
    {
    case cs_LockedForReading:
      out << "\"Locked For Reading\"";
      break;
    case cs_LockedForWriting:
      out << "\"Locked For Writing\"";
      break;
    case cs_AvailableForNextWriting:
    default:
      out << "\"Available For Next Writing\"";
      break;
    }
    out
      << std::endl
      << "data : " << (int)(buffTripleContainer[i].GetBuffer()->operator[](0)) 
      << std::endl
      << std::endl;
  }
}

CTripleBuffContainer::CTripleBuffContainer() : 
  bCreated(false), 
  eState(cs_AvailableForNextWriting), 
  eStatePre(cs_AvailableForNextWriting) { }

CTripleBuffContainer::~CTripleBuffContainer() { }

void CTripleBuffContainer::Create(int par_nColSize, int par_nRowSize, int par_nDefaultValue) {
  if (bCreated) return;
  buff.Create(par_nColSize, par_nRowSize);
  buff.setAllBytes(par_nDefaultValue);
  bCreated = true;
}

void CTripleBuffContainer::Destroy() {
  buff.Destroy();
}

CBuffer * CTripleBuffContainer::GetBuffer() const {
  if (!bCreated) _ASSERT(false);
  return &buff;
}

teContainerState CTripleBuffContainer::GetState() const {
  return eState;
}

bool CTripleBuffContainer::HasDataWrittenAndReadyToRead() {
  if (eState == cs_AvailableForNextWriting && eStatePre == cs_LockedForWriting) return true;
  return false;
}

void CTripleBuffContainer::SetState(const teContainerState& par_eState) {
  if (!bCreated) _ASSERT(false);
  eStatePre = eState;
  eState = par_eState;
}

CTripleBufferManager::CTripleBufferManager(int par_nColumnSize, int par_nRowSize, int par_nDefaultValue) : 
  nReadingBuffIndex(0),
  nWritingBuffIndex(0),
  buffTripleContainer(BUFFER_CONTAINER_SIZE) {

  // initialize cbuff containers for given depth
  for (int i=0; i < BUFFER_CONTAINER_SIZE; ++i) 
  {
    buffTripleContainer[i].Create(par_nColumnSize, par_nRowSize, par_nDefaultValue);   
    buffTripleContainer[i].SetState(cs_AvailableForNextWriting);
  }
}

CTripleBufferManager::~CTripleBufferManager() {
  for (int i=0; i<BUFFER_CONTAINER_SIZE; ++i) 
  {
    buffTripleContainer[i].Destroy();
  }
}

CBuffer * CTripleBufferManager::GetNewReadingBuffer() {
  int nAvailableBuffIndex = GetIndexForFirstAvailableForReading();

  // If there is no available frame
  if(nAvailableBuffIndex < 0) return nullptr;
                 
  buffTripleContainer[nReadingBuffIndex].SetState(cs_AvailableForNextWriting);
    
  buffTripleContainer[nAvailableBuffIndex].SetState(cs_LockedForReading);
  nReadingBuffIndex = nAvailableBuffIndex;  
    
  return buffTripleContainer[nReadingBuffIndex].GetBuffer();
}

CBuffer * CTripleBufferManager::GetNewWritingBuffer() {    
  // set currently writing state buffer as ready for reading
  int nAvailableBuffIndex = GetIndexForFirstAvailableForWriting();
  _ASSERT(nAvailableBuffIndex >= 0);
    
  buffTripleContainer[nWritingBuffIndex].SetState(cs_AvailableForNextWriting);
    
  buffTripleContainer[nAvailableBuffIndex].SetState(cs_LockedForWriting);
  nWritingBuffIndex = nAvailableBuffIndex;                        
    
  return buffTripleContainer[nWritingBuffIndex].GetBuffer();
}

int CTripleBufferManager::GetIndexForFirstAvailableForReading() {
  std::lock_guard<decltype(itsCMutex)> lock(itsCMutex);
  for (int i=0; i<BUFFER_CONTAINER_SIZE; ++i)
  {
    // buffer elemanin state'i available olmali ve okunmak 
    if (buffTripleContainer[i].HasDataWrittenAndReadyToRead()) return i;
  }
  return -1;
}

int CTripleBufferManager::GetIndexForFirstAvailableForWriting() {
  std::lock_guard<decltype(itsCMutex)> lock(itsCMutex);
  for (int i=0; i<BUFFER_CONTAINER_SIZE; ++i)
  {
    // buffer elemanin state'i available olmali ve okunmak 
    if (buffTripleContainer[i].GetState() == cs_AvailableForNextWriting) return i;
  }
  return -1;
}
