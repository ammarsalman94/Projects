/////////////////////////////////////////////////////////////////////
// ThreadPool.cpp - Provides implementation for ThreadPool.h       //
// ver 1.0                                                         //
//                                                                 //
// Ammar Salman, CSE687 - Object Oriented Design, Spring 2017      //
//               (313) 788 4694 - hoplite.90@hotmail.com           //
//               Syracuse University, EECS Department              //
/////////////////////////////////////////////////////////////////////

#include "ThreadPool.h"

// -----< Default Constructor > ----------------------------------------------
/* Initializes the ThreadPool with the current number of cores found */
ThreadPool::ThreadPool() : ThreadCount_(std::thread::hardware_concurrency()){
  Initialize();
}

// -----< Constructor >-------------------------------------------------------
/* Initializes the ThreadPool with user-specific number of threads */
ThreadPool::ThreadPool(const size_t & numThreads) : ThreadCount_(numThreads){
  Initialize();
}

// -----< Destructor >--------------------------------------------------------
ThreadPool::~ThreadPool() {
  Terminate();
}

// -----< Get Current Number of Running Threads >-----------------------------
const size_t & ThreadPool::ThreadCount() const {
  return ThreadCount_;
}

// -----< Add Job to the Queue (void func()) >---------------------------------
void ThreadPool::AddJob(std::function<void()>& Job) {
  // wrap the void job(void) with bool ex(void) for the threadpool functionality
  std::function<bool()> executable = [Job]()->bool {
    Job();
    return true;
  };
  Q_.enQ(executable);
}

// -----< Extend Number of ThreadPool Threads >-------------------------------
void ThreadPool::Extend(size_t Extension) {
  wmtx.lock();
  ThreadCount_ += Extension;
  for (size_t i = ThreadCount_-Extension; i < ThreadCount_; ++i) {
    ThreadPoolActive_.push_back(true);
    std::thread t([this, i] { ThreadProc(i); });
    ThreadPool_.push_back(std::move(t));
  }
  wmtx.unlock();
}

// -----< Shrink Number of ThreadPool Threads >--------------------------------
bool ThreadPool::Shrink(size_t Shrink) {
  wmtx.lock();
  // keep at least 1 worker thread in the pool
  if (Shrink >= ThreadCount_) {
    wmtx.unlock();
    return false;
  }
  for (size_t i = ThreadCount_ - 1; i >= ThreadCount_ - Shrink; --i) {
    ThreadPoolActive_[i] = false;
    if (ThreadPool_[i].joinable())
      ThreadPool_[i].join();
    ThreadPool_.pop_back();
    ThreadPoolActive_.pop_back();
  }
  ThreadCount_ -= Shrink;
  wmtx.unlock();
  return true;
}

// -----< Terminate ThreadPool >-----------------------------------------------
void ThreadPool::Terminate() {
  WatchEnabled_ = false;
  if (Watch_.joinable())
    Watch_.join();
  Q_.enQ(QuitJob_);
  wmtx.lock();
  for (size_t i = 0; i < ThreadCount_; ++i) {
    if (ThreadPool_[i].joinable())
      ThreadPool_[i].join();
  }
  ThreadPool_.clear();
  ThreadPoolActive_.clear();
  ThreadCount_ = 0;
  wmtx.unlock();
  Q_.clear();
}

// -----< ReInitialize ThreadPool >--------------------------------------------
/* This can only be called after Termination */
bool ThreadPool::ReInitialize(size_t numThreads) {
  if (ThreadPool_.size() > 0) return false;
  ThreadCount_ = numThreads;
  Initialize();
  return true;
}

// -----< Get ThreadPool Shared Mutex >----------------------------------------
std::mutex & ThreadPool::mutex() {
  return mtx_;
}

// -----< Check if Watch Thread is Working >-----------------------------------
const bool & ThreadPool::Watch() const {
  return WatchEnabled_;
}

// -----< Turn on/off Watch Thread >-------------------------------------------
void ThreadPool::Watch(const bool & watch) {
  if (LongDuration_ == 0)
    LongDuration_ = 500; // if no duration specified, put it to 500ms as default
  WatchEnabled_ = watch;
  if (watch)
    Watch_ = std::thread(&ThreadPool::WatchThreadProc, this);
  else if (Watch_.joinable())
    Watch_.join();
}

// -----< Get the current set "long" duration >-------------------------------
const size_t & ThreadPool::LongDuration() const {
  return LongDuration_;
}

// -----< Set "long" duration value (must be > 0) >---------------------------
void ThreadPool::LongDuration(const size_t & dur) {
  if (dur > 0)
    LongDuration_ = dur;
}

// -----< Initialize ThreadPool >---------------------------------------------
void ThreadPool::Initialize() {
  // set QuitJob to return false
  QuitJob_ = []() -> bool {
    return false;
  };
  for (size_t i = 0; i < ThreadCount_; ++i) {
    ThreadPoolActive_.push_back(true);
    std::thread t([this, i] { ThreadProc(i); });
    ThreadPool_.push_back(std::move(t));
  }
}

// -----< Worker Thread Processing Loop >---------------------------------------
/* threadNumber is the thread index in the ThreadPool_ vector*/
void ThreadPool::ThreadProc(size_t threadNumber) {
  while (ThreadPoolActive_[threadNumber]) {
    std::function<bool()> executable = Q_.deQ();
    // QuitJob will force break despite ThreadPoolActive_[threadNumber] == true
    if (!executable()) {
      Q_.enQ(executable);
      break;
    }
  }
}

// -----< Watch Thread Processing Loop >------------------------------------------
void ThreadPool::WatchThreadProc() {
  bool isBusy = false;   // indicates whether or not the queue is "full"
  size_t Extension = 0;  // tells the thread how many times it extended

  // by default, WatchEnabled_ = false so this thread will be down
  while (WatchEnabled_) {
    // if the queue is full since over the LongDuration, spawn a new thread
    if (Q_.size() >= ThreadCount_ && isBusy) {
      Extend(++Extension);
      isBusy = false; // reset busy record to make this wait at least 
                      // twice the longduration before spawning another thread
    }
    // if it was not full since last measured time, but is full now
    // set isBusy to true, so next time the check is made we have record
    else if (Q_.size() >= ThreadCount_)
      isBusy = true;
    // if there was extension, but the queue has less jobs than threads
    // terminate one of the spawned worker threads. This does not 
    // terminate original threads that were initialized on construction
    else if (Extension > 0 && Q_.size() < ThreadCount_ - Extension) {
      isBusy = false;
      Shrink(1);
      Extension--;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(LongDuration_));
  }
}


#ifdef TEST_THREAD_POOL

int main() {
  std::cout << "\n  Initializing ThreadPool and pushing jobs to it";
  ThreadPool threadPool;
  threadPool.LongDuration(2000);
  threadPool.Watch(true);
  std::function<void()> LongJob = [&threadPool]() {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    threadPool.mutex().lock();
    std::cout << "\n    Long job on thread " << std::this_thread::get_id() << " is finished";
    threadPool.mutex().unlock();
  };

  std::function<void()> Job = [&threadPool]() {
    threadPool.mutex().lock();
    std::cout << "\n    Hello from thread " << std::this_thread::get_id();
    threadPool.mutex().unlock();
  };
  for (int i = 0; i < 4; i++) {
    threadPool.AddJob(LongJob);
  }
  for (int i = 0; i < 12; i++) {
    threadPool.AddJob(Job);
  }
  for (int i = 0; i < 4; i++) {
    threadPool.AddJob(LongJob);
  }
  // terminate will terminate the watch thread, so
  // let main thread sleep before calling terminate in  
  // order to allow the Watch thread to spawn thread(s)
  std::this_thread::sleep_for(std::chrono::seconds(5));

  // terminate is a blocking call until all threadpool threads are done executing
  threadPool.Terminate();
  for (int i = 0; i < 12; i++) {
    threadPool.AddJob(Job);
  }
  std::cout << "\n\n\n  Reinitializing ThreadPool and pushing jobs to it";
  threadPool.ReInitialize();
  
  threadPool.Terminate();
  std::cout << "\n\n";
  return 0;
}


#endif