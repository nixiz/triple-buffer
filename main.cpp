#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include "CTripleBufferManager.h"

// global mutex for synchronizing print outs 
std::mutex guard;

void printvalues(const CTripleBufferManager& manager) {
  std::lock_guard<std::mutex> lck(guard);
  static int _times = 0;
  std::cout 
    << "--------------------------------------------------"
    << std::endl
    << "step: " << ++_times << std::endl << std::endl;
  manager.print_sync(std::cout);
}

int main(int argc, char const *argv[])
{
  /* code */
  CTripleBufferManager manager(1, 1);
  std::atomic_bool done(false);
  using namespace std::chrono_literals;

  // thread for writing into buffer
  std::thread t([&] {
    while (!done)
    {
      static int _counter = 0;
      auto *buff = manager.GetNewWritingBuffer();
      buff->setAllBytes(++_counter);
      printvalues(manager);
      std::this_thread::sleep_for(100ms);
    }
  });
  t.detach();

  // main thread for reading non-empty slots
  while (!done)
  {
    auto *buff = manager.GetNewReadingBuffer();

    if (buff != nullptr)
    {
      std::lock_guard<std::mutex> lck(guard);
      std::cout << "current read slot: " << (int)(*buff)[0] << std::endl;
      std::cout << "press enter to read another slot.." << std::endl;
    }
    //getchar();
    std::this_thread::sleep_for(200ms);
  }
  return 0;
}
