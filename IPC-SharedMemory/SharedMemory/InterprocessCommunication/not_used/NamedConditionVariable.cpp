#include "NamedConditionVariable.h"

#include <exception>

ip_condition_variable::ip_condition_variable() : mWaiters(0), mLock(NULL), mSem(NULL)
{
}

ip_condition_variable::~ip_condition_variable()
{
  CloseHandle(mSem);
  CloseHandle(mLock);
}

const std::string & ip_condition_variable::name()
{
  return mName;
}

// -----< Create named semaphores >------------------------------------------------
bool ip_condition_variable::initialize(const std::string & name)
{
  mName = name;

  mSem = CreateSemaphore(NULL, 0, INFINITE, name.c_str());

  std::string lockName = name + "_Lock";
  mLock = CreateSemaphore(NULL, 0, 1, lockName.c_str());

  if (!mSem || !mLock) return FAILURE;

  return SUCCESS;
}

// -----< Connect to existing named semaphores >-----------------------------------
bool ip_condition_variable::connect_to(const std::string & name)
{
  mName = name;
  mSem = OpenSemaphore(SEMAPHORE_MODIFY_STATE, FALSE, name.c_str());

  std::string lockName = name + "_Lock";
  mLock = OpenSemaphore(SEMAPHORE_MODIFY_STATE, FALSE, lockName.c_str());

  if (!mSem || !mLock) return FAILURE;

  return SUCCESS;
}

void ip_condition_variable::close()
{
  CloseHandle(mSem);
  CloseHandle(mLock);
  mWaiters = 0;
}

// -----< Wait on the semaphores >-------------------------------------------------
bool ip_condition_variable::wait(unsigned int waitTimeMs)
{
  WaitForSingleObject(mLock, INFINITE);  // Lock
  mWaiters++;                             // Add to wait count
  ReleaseSemaphore(mLock, 1, NULL);      // Unlock 

                                         // This must be outside the lock
  return (WaitForSingleObject(mSem, waitTimeMs) == WAIT_OBJECT_0);
}

// -----< Signal one waiter on mSem >----------------------------------------------
bool ip_condition_variable::signal()
{
  WaitForSingleObject(mLock, INFINITE);           // Lock
  mWaiters--;                                      // Lower wait count
  bool result = ReleaseSemaphore(mSem, 1, NULL); // Signal 1 waiter 
  ReleaseSemaphore(mLock, 1, NULL);               // Unlock 
  return result;
}

// -----< Signal all currently waiting waiters >-----------------------------------
bool ip_condition_variable::broadcast()
{
  WaitForSingleObject(mLock, INFINITE);                 // Lock 
  bool result = ReleaseSemaphore(mSem, mWaiters, NULL); // Signal all
  mWaiters = 0;                                          // All waiters clear;
  ReleaseSemaphore(mLock, 1, NULL);                     // Unlock 
  return result;
}


