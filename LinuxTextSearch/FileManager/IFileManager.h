///////////////////////////////////////////////////////////////////
// IFileManager.h : Defines IFileManager interface               //
//                                                               //
// Platform:     Dell Inspiron 5520, Ubuntu 16.04, NetBeans 8.2  //
// Application:  Distributed Objects - CSE-775, Project 1 Sp17   //
// Author:       Ammar Salman, Syracuse University               //
//               (313) 788-4694, hoplite.90@hotmail.com          //
///////////////////////////////////////////////////////////////////
/*
 * This package defines IFileManager interface which acts similar
 * to COM interfaces. This interface exposes the necessary methods
 * that we want clients to use. The functions are pure virtual
 * indicating that we cannot create instance of IFileManager. 
 * It is intended to be used as FileManager derived class.
 * 
 */
/*
 * Public interface:
 * IFileManager* fmgr = new FileManager;
 * fmgr->ConfigureSearcher(txtSearcher);
 * fmgr->SetWorkingDirectory("/home/ammar/test/", "*.txt", true);
 * fmgr->SetSearchString("ammar");
 * fmgr->PerformOperations();
 * fmgr->Clear();
 * // the results are obtained from txtSearcher instance
 * 
 * Required Files:
 *  - IFileManager.h
 *  - IFileManager.cpp 
 *  - ITextSearch.h
 * 
 * Build Process:
 *  g++    -c -g -s -std=c++14 -fPIC  -MMD -MP -MF 
 *     "build/Debug/GNU-Linux/FileSystem.o.d" 
 *     -o build/Debug/GNU-Linux/FileSystem.o FileSystem.cpp 
 * 
 * Maintenance History:
 * ver 1.0
 *  - first release
 */

#pragma once

#if defined _WIN32 || defined __CYGWIN__
  #ifdef BUILDING_DLL
    #ifdef __GNUC__
      #define DLL_PUBLIC __attribute__ ((dllexport))
    #else
      #define DLL_PUBLIC __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #else
    #ifdef __GNUC__
      #define DLL_PUBLIC __attribute__ ((dllimport))
    #else
      #define DLL_PUBLIC __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #endif
  #define DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define DLL_PUBLIC __attribute__ ((visibility ("default")))
    #define DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define DLL_PUBLIC
    #define DLL_LOCAL
  #endif
#endif

#include <string>
#include "../TextSearch/ITextSearch.h"

class IFileManager {
public:
    virtual ~IFileManager(void){}
    static IFileManager* CreateFileManager(void);
    virtual void ConfigureSearcher(ITextSearch*) = 0;
    virtual void SetWorkingDirectory(std::string, std::string, bool) = 0;
    virtual void SetSearchString(std::string) = 0;
    virtual void PerformOperations() = 0;
    virtual void Clear() = 0;
};

/* using extern "C" is more of making a promise not to
 * overload the function which allows for calling it
 * by its name when the library is loaded using `dlopen` */
extern "C" {
    DLL_PUBLIC IFileManager* CreateFileMgrInstance();
}

