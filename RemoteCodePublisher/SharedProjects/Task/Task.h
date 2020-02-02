#pragma once
/////////////////////////////////////////////////////////////////////
// Tasks.h - Provides Tasks class and template Task class          //
// ver 1.0                                                         //
//                                                                 //
// Ammar Salman, CSE687 - Object Oriented Design, Spring 2017      //
//               (313) 788 4694 - hoplite.90@hotmail.com           //
//               Syracuse University, EECS Department              //
/////////////////////////////////////////////////////////////////////
/*
* There are two classes. Base class Tasks that holds static field 
* representing the ThreadPool, and static functions. It is declared
* with a pure virtual function to disable class instantiation.
* 
* The template class Task<typename ReturnType> derives Tasks base,
* this way any Task of any type will be using the same ThreadPool,
* e.g. Task<int>, Task<bool>, ..., etc. will all push tasks to the 
* same ThreadPool.
*
* Tasks can be directly used to push [ void function (void) ] 
* which will be pushed to the ThreadPool to execute. Tasks
* class cannot be instantiated.
*
* Task<typname ReturnType> must be instantiated, and if it was
* started, it must wait for the result, whether explictly by
* the programmer, or implicitly if the programmer forgot to
* get the result back. Implicit wait is called upon destruction
* to ensure the Task is not destroyed before its processing is
* finished.
*
* Using Tasks, it is possible set the number of Threads in the
* ThreadPool, it is also possible to get the mutex to ensure
* thread safety upon Tasks.
*
*
* Public Interface:
* Tasks::Delay(500); // makes caller thread sleep for 500ms
* Tasks::Start([]{ 
*     // do stuff
*   });
* Tasks::ConcurrentTasks(8); // sets ThreadPool size to 8
* Tasks::ConcurrentTasks();  // returns ThreadPool size
* Tasks::Mutex() // returns ThreadPool's mutex
* 
* Task<int> task([]{ return 1; }); // Task with dummy function returning 1
* task.Start(); // push task to the ThreadPool
* task.Wait();  // wait until task is finished
* int result = task.Result();    // result = 1
*
*/
/*
* Required Files:
* --------------------
*  Task.h Task.cpp ThreadPool.h ThreadPool.cpp
*
* Build Process:
* --------------------
* devenv Task.vcxproj /rebuild debug

* Maintenance History:
* --------------------
* ver 1.0 - April 22nd 2017
*   -first release
*
*/

#include "..\ThreadPool\ThreadPool.h"
#include <atomic>

// =====< Tasks Class Declarations >===========================================
class Tasks {
public:
  virtual void Start() = 0;
  static void Start(std::function<void()> Job);
  static void Delay(long long Milliseconds);
  static void ConcurrentTasks(size_t numTasks);
  static std::mutex& Mutex();
  static size_t ConcurrentTasks();
  static void WaitAllTasks();
protected:
  static ThreadPool threadPool_;
};


// ============================================================================

// =====< Task Template Class Declarations >===================================

template<typename ReturnType>
class Task : public Tasks {
public:
  Task();
  Task(std::function<ReturnType()> func);
  ~Task();

  void PutTask(std::function<ReturnType()> Job);
  virtual void Start() override;
  bool Wait();
  const ReturnType& Result() const;

private:
  ReturnType data_;
  std::atomic_bool resultReady;
  bool started;
  std::function<void()> func;
};

// ============================================================================

// -----< Default Constructor >------------------------------------------------
template<typename ReturnType>
inline Task<ReturnType>::Task() : resultReady(false) {
}


// -----< Constructor Using Function >-----------------------------------------
template<typename ReturnType>
inline Task<ReturnType>::Task(std::function<ReturnType()> func) : resultReady(false) {
  PutTask(func);
}

// -----< Destructor >---------------------------------------------------------
template<typename ReturnType>
inline Task<ReturnType>::~Task() {
  /* If the Task was destroyed before its processing has been finished
   * then the ThreadPool would be trying to access invalid memory
   * location and hence causing the program to crash.
   * Therefore, before the Task is destroyed, it should wait
   * for its result to be ready. This makes sense because Task is 
   * designed to return results. If the user didnt want any results
   * back and only wanted the processing, the base class Tasks
   * already does the job. E.g. Tasks::Start(lambda); */
  Wait();
}

// -----< Prepare Task for the ThreadPool >------------------------------------
template<typename ReturnType>
inline void Task<ReturnType>::PutTask(std::function<ReturnType()> Job) {
  func = [this, Job] {
      this->data_ = Job();
      this->resultReady = true;
  };
}

// -----< Push Task to the ThreadPool >----------------------------------------
template<typename ReturnType>
inline void Task<ReturnType>::Start() {
  started = true;
  Tasks::threadPool_.AddJob(func);
}

// -----< Wait for Task to finish >--------------------------------------------
template<typename ReturnType>
inline bool Task<ReturnType>::Wait() {
  if (!started) return false;
  while (!resultReady) std::this_thread::yield();
  // reset Task status, but do not clear the result
  started = false;
  return true;
}

// -----< Get Result Back >----------------------------------------------------
template<typename ReturnType>
inline const ReturnType & Task<ReturnType>::Result() const {
  return data_;
}
