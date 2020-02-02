///////////////////////////////////////////////////////////////////
// ITextSearch.cpp : Defines TextSearch class                    //
//                                                               //
// Platform:     Dell Inspiron 5520, Ubuntu 16.04, NetBeans 8.2  //
// Application:  Distributed Objects - CSE-775, Project 1 Sp17   //
// Author:       Ammar Salman, Syracuse University               //
//               (313) 788-4694, hoplite.90@hotmail.com          //
///////////////////////////////////////////////////////////////////
/*
 * This package defines TextSearch class that inherets from inter-
 * face ITextSearch and implements its functions. The client does
 * not directly interact with this class. It is done through the
 * ITextSearch interface which returns pointer to this.
 * 
 */
/*
 * Public interface:
 * ITextSearch* txtSearcher = new TextSearch;
 * txtSearcher->PutFile("/home/ammar/test/test.txt", "ammar");
 * 
 * 
 * Required Files:
 *  - ITextSearch.h
 *  - ITextSearch.cpp 
 *  - Cpp11-BlockingQueue.h
 * 
 * Build Process:
 *  g++    -c -g -s -std=c++14 -fPIC  -MMD -MP -MF 
 *     "build/Debug/GNU-Linux/ITextSearch.o.d" 
 *     -o build/Debug/GNU-Linux/ITextSearch.o ITextSearch.cpp 
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

using Pair = std::pair<std::string, std::string>;
using Record = std::pair<Pair, bool>;
    
class ITextSearch {
public:
    virtual ~ITextSearch(void){}
    static ITextSearch* CreateTextSearch(void);
    virtual void PutFile(std::string, std::string) = 0;
    virtual Record GetRecord() = 0;
};

/* using extern "C" is more of making a promise not to
 * overload the function which allows for calling it
 * by its name when the library is loaded using `dlopen` */
extern "C" {
    DLL_PUBLIC ITextSearch* CreateITextSearch();
}


