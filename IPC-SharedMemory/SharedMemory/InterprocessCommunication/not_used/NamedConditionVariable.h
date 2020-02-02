#pragma once

// for interprocess communication
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>

#include <string>

#define SUCCESS 1
#define FAILURE 0

class ip_condition_variable {
private:
  HANDLE  mSem;        // Used to signal waiters
  HANDLE  mLock;       // Semaphore used as inter-process lock 
  int     mWaiters;    // # current waiters 

  std::string mName;

public:
  ip_condition_variable();
  virtual ~ip_condition_variable();

  const std::string& name();

  bool initialize(const std::string& name);
  bool connect_to(const std::string& name);
  void close();

  bool signal();
  bool broadcast();
  bool wait(unsigned int waitTimeMs = INFINITE);
};

