/////////////////////////////////////////////////////////////////////
// Tasks.cpp - Provides definition only for base class Tasks       //
// ver 1.0                                                         //
//                                                                 //
// Ammar Salman, CSE687 - Object Oriented Design, Spring 2017      //
//               (313) 788 4694 - hoplite.90@hotmail.com           //
//               Syracuse University, EECS Department              //
/////////////////////////////////////////////////////////////////////

#include "Task.h"

ThreadPool Tasks::threadPool_;

// -----< Push Task to the ThreadPool directly >-----------------------
void Tasks::Start(std::function<void()> Job) {
  Tasks::threadPool_.AddJob(Job);
}

// -----< Block Caller Thread for Milliseconds >-----------------------
void Tasks::Delay(long long Milliseconds) {
  std::this_thread::sleep_for(std::chrono::milliseconds(Milliseconds));
}

// -----< Set ThreadPool Size >----------------------------------------
void Tasks::ConcurrentTasks(size_t numTasks) {
  if (Tasks::threadPool_.ThreadCount() > numTasks) {
    // if threadpool had 4 threads running and we want to set them 
    // to 8 then we extend by 4, i.e. extend by the difference
    Tasks::threadPool_.Extend(Tasks::threadPool_.ThreadCount() - numTasks);
  }
  else if (Tasks::threadPool_.ThreadCount() < numTasks) {
    // if threadpool had 8 threads running and we want to set them
    // to 4 then we shrink by 4, i.e. shrink by the difference
    Tasks::threadPool_.Shrink(numTasks - Tasks::threadPool_.ThreadCount());
  }
  // else means threadpool thread count is the same as numTaks, so do nothing
}

// -----< Return ThreadPool Mutex >------------------------------------
std::mutex & Tasks::Mutex() {
  return Tasks::threadPool_.mutex();
}

// -----< Get ThreadPool Size >----------------------------------------
size_t Tasks::ConcurrentTasks() {
  return Tasks::threadPool_.ThreadCount();
}

// -----< Wait for all Tasks to Finish >-------------------------------
void Tasks::WaitAllTasks() {
  size_t numThreads = Tasks::threadPool_.ThreadCount();
  Tasks::threadPool_.Terminate();
  Tasks::threadPool_.ReInitialize(numThreads);
}


#ifdef TEST_TASKS

int main() {
  std::vector<Task<int>> tasks(10);
  for (int i = 0; i < 10; i++) {
    tasks[i].PutTask([i] {
      Tasks::Mutex().lock();
      Tasks::Delay(500);
      std::cout << "\n  Hi from Task<int> #" << i;
      Tasks::Mutex().unlock();
      return 10; });
    tasks[i].Start();
  }
  std::vector<Task<std::string>> stasks(10);
  for (int i = 0; i < 10; i++) {
    stasks[i].PutTask(([i] {
      Tasks::Delay(100);
      Tasks::Mutex().lock();
      std::cout << "\n  Hi from Task<string> #" << i;
      Tasks::Mutex().unlock();
      return "ammar"; }));
    stasks[i].Start();
  }
  for (int i = 0; i < 10; ++i) {
    tasks[i].Wait();
    stasks[i].Wait();
    Tasks::Mutex().lock();
    std::cout << "\n   Task<int> #" << i << " returned: " << tasks[i].Result();
    std::cout << "\n   Task<int> #" << i << " returned: " << stasks[i].Result();

    // Task was already waited for and so it resets after wait
    // this wait will always return false
    //if (tasks[i].Wait())
    //  std::cout << "\n   Task<int> #" << i << " returned: " << tasks[i].Result();
    //else
    //  std::cout << "\n   Task<int> #" << i << " already returned result: " << tasks[i].Result();
    Tasks::Mutex().unlock();
  }

}

#endif // TEST_TASKS
