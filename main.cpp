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
  std::cout << "step: " << ++_times << std::endl << std::endl;
  manager.print_sync(std::cout);
}

int main(int argc, char const *argv[])
{
  /* code */
  CTripleBufferManager manager(1, 1);
  std::atomic_bool done(false);

  // thread for writing into buffer
  std::thread t([&] {
    using namespace std::chrono_literals;
    while (!done)
    {
      static int _counter = 0;
      auto *buff = manager.GetNewWritingBuffer();
      buff->setAllBytes(++_counter);
      printvalues(manager);
      std::this_thread::sleep_for(1000ms);
    }
  });
  t.detach();

  // main thread for reading non-empty slots
  while (!done)
  {
    auto *buff = manager.GetNewReadingBuffer();

    if (buff != nullptr)
    {
      guard.lock();
      std::cout << "current read slot: " << (int)(*buff)[0] << std::endl;
      std::cout << "press enter to read another slot..";
      guard.unlock();
    }
    getchar();
  }
  return 0;
}
