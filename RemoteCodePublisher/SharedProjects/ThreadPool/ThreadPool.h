#pragma once
/////////////////////////////////////////////////////////////////////
// ThreadPool.h - Enqueue work items to run on one of a collection //
//                of threads, created at startup.                  //
// ver 1.0                                                         //
//                                                                 //
// Ammar Salman, CSE687 - Object Oriented Design, Spring 2017      //
//               (313) 788 4694 - hoplite.90@hotmail.com           //
//               Syracuse University, EECS Department              //
/////////////////////////////////////////////////////////////////////
/*
* Threadpool is a collection of number of threads that accept jobs
* and execute them. The default constructor will initialize threads
* as many cores found on the current machine.
*
* The queue accepts jobs that return boolean. However, the class
* accepts jobs that take no arguments and return nothing. Then it
* wraps the passed job with a boolean function that returns true. 
* I have chosen this design so if the user forgot to put quit 
* messgae, the destructor will automatically do it.
* This crucial for the Tasks class since it cannot be instantiated
* and so there is no destructor. To make the user able just to 
* push tasks using Tasks class and let them go, ThreadPool will
* have to destory itself upon release of all resources.
*
* When ordered to terminate, the terminate method enQs the quit
* message (which only returns false) to the queue, whenever a thread
* receives that message, it will put it back to the queue, allowing
* other threads to read it, and then it will break.
*
* I have added functionality when there are long jobs monopolizing
* the threadpool for themselves. I have added a thread called the 
* Watch_, it is disabled by default. When enabled, it checks whether
* the number of jobs in the queue exceeds the number of running 
* threads over the "long" duration. If it does, the watch thread 
* will spawn another thread to take jobs from the queue. 
*
* Moreover, if the jobs where finished the watch thread will terminate
* the spawned thread(s) until it reaches the original number of threads.
* 
* There are really two ways to terminate threads, not only one. The
* first way is using the quit message as described above. This method
* allows the threadpool to finish all the jobs before it terminates.
* This termination will block until all jobs have finished.
*
* However, the watch thread will have to terminate selected threads
* if there was no need for them anymore. Therefore, I have chosen to
* add boolean vector that tells the corresponding thread's work. 
* This way, the watch thread will be able to terminate selected
* threads without sending a quit message and killing the pool.
*
* I have added the functionality to allow the user to manually extend
* the total number of threads in the threadpool given the extension
* size. Similarly, I have added Shrinking functionality for the user
* to be able to reduce the number of working threads, if desired.
*
* There are two Mutex'es, one to be shared ONLY by the worker threads;
* the other Mutex is shared by the Watch thread and any thread that
* calls Extend, Shrink or Terminate. The need for the second Mutex is
* to ensure the correctness of ThreadCount. Imagine the Watch thread
* was spawning a new thread, in the same time, the user asked for extension
* of 4 more threads, only one of the two operations should be allowed at
* a time. The same applies to any combination of the three specified 
* operations (Extend, Shrink and Terminate).
*
*
* Public Interface:
* ThreadPool threadPool; // ThreadCount is the number of cores on the machine
* ThreadPool threadPool(8); // ThreadCount = 8
* threadPool.Extend(4); // adds 4 more threads to the pool
* threadPool.Shrink(4); // removes 4 threads from the pool
* threadPool.AddJob([]{ std::cout << "Hi";}); // adds job to the pool
* threadPool.mutex(); // returns mutex to enable the user to add thread safety
* threadPool.Terminate(); // terminates all working threads
* threadPool.ReInitialize();
*/
/*
* Required Files:
* --------------------
* ThreadPool.h ThreadPool.cpp Cpp11-BlockingQueue.h Cpp11-BlockingQueue.cpp
*
* Build Process:
* --------------------
* devenv ThreadPool.vcxproj /rebuild debug

* Maintenance History:
* --------------------
* ver 1.0 - April 22nd 2017
*   -first release
*
*/

#include <vector>
#include <functional>
#include <thread>

#include "..\Cpp11-BlockingQueue\Cpp11-BlockingQueue.h"

class ThreadPool {
public:
  ThreadPool();
  ThreadPool(const size_t& numThreads);
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;
  ~ThreadPool();

  void AddJob(std::function<void()>& Job);
  
  const size_t& ThreadCount() const;
  void Extend(size_t Extension);
  bool Shrink(size_t Shrink);

  void Terminate();
  bool ReInitialize(size_t numThreads = std::thread::hardware_concurrency());
  
  std::mutex& mutex();

  const bool& Watch() const;
  void Watch(const bool& watch);
  const size_t& LongDuration() const;
  void LongDuration(const size_t& dur);

private:

  size_t ThreadCount_;
  std::vector<std::thread> ThreadPool_;
  std::vector<bool> ThreadPoolActive_;
  BlockingQueue<std::function<bool()>> Q_;
  std::function<bool()> QuitJob_;

  std::mutex mtx_;
  std::mutex wmtx;

  bool WatchEnabled_;
  size_t LongDuration_;
  std::thread Watch_;

  void Initialize();
  void ThreadProc(size_t threadNumber);
  void WatchThreadProc();
};
