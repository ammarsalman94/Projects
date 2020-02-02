///////////////////////////////////////////////////////////////////
// TextSearchClient.cpp : Client for TextSearch                  //
//                               and FileManager Libraries       //
//                                                               //
// Platform:     Dell Inspiron 5520, Ubuntu 16.04, NetBeans 8.2  //
// Application:  Distributed Objects - CSE-775, Project 1 Sp17   //
// Author:       Ammar Salman, Syracuse University               //
//               (313) 788-4694, hoplite.90@hotmail.com          //
///////////////////////////////////////////////////////////////////

#include <dlfcn.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <exception>
#include <thread>
#include "../TextSearch/ITextSearch.h"
#include "../FileManager/IFileManager.h"


void ShowUsage(){
    std::cout << "\n Usage: --path /path/to/dir --search \"example search\"";
    std::cout << " [--patterns \"*.*\" -subs]";
    std::cout << "\n  --path:     specifies working directory path (must be set)";
    std::cout << "\n  --search:   specifies string to search - (must be set)";
    std::cout << "\n  --patterns: specifies file extension patterns (default: *.*)";
    std::cout << "\n  -subs:      if specified the program includes sub-directories\n";
}

bool ProcessCommandLineArgs(int argc, char** argv, std::string* wPath, 
        std::string* toSearch, std::string* Patterns, bool* incSubs){
    *wPath = "dummy";
    *toSearch = "dummy";
    *Patterns = "*.*";
    for(int i=1; i<argc; i++){
        std::string arg(argv[i]);
        if(arg=="--path"){
            *wPath = argv[i+1];
        }
        if(arg=="--patterns"){
            *Patterns = argv[i+1];
        }
        if(arg=="--search"){
            *toSearch = argv[i+1];
        }
        if(arg=="-subs"){
            *incSubs = true;
        }
    }
    if(*wPath=="dummy" || *toSearch=="dummy") return false;
    return true;
}


void ThreadProc(ITextSearch* txtSearcher){
    std::cout << "\n\n  Started Results Receiver Thread ";
    std::cout << "\n =================================";
    
    while(true){
    auto rec = txtSearcher->GetRecord();
    if(rec.first.first == "quit"){
        std::cout << "\n\n  Received quit message. Terminating receiver thread.. \n\n";
        break;
    }
    std::cout << "\n  Searched for: '" << rec.first.second << "' -- Found: " 
            << std::boolalpha << rec.second << " -- File: " << rec.first.first;
    }
}

int main(int argc, char** argv) {
    std::string wPath, toSearch, Patterns; bool incSubs;
    if(!ProcessCommandLineArgs(argc, argv, 
            &wPath, &toSearch, &Patterns, &incSubs)){
        ShowUsage();
        return 1;
    }
    std::cout << "\n  Starting Text Search Client";
    std::cout << "\n =============================\n";
    
    // there is a special need to explicitly load pthread library
    // so that TextSearch library could operate correctly as it needs pthread
    void* handlePthread = dlopen("libpthread.so.0", RTLD_GLOBAL | RTLD_LAZY);
    if(!handlePthread ){
        std::cout << "\n  dlopen failed: " <<  dlerror() << "\n\n";
        return 1;
    }
    std::cout << "\n  Loaded pthread library successfully";
    
    // libTextSearch.so should be placed in /usr/lib
    // using "sudo cp libTextSearch.so /usr/lib" should be enough
    void* handle = dlopen("libTextSearch.so", RTLD_LAZY);
    if(!handle){
        std::cout << "\n  dlopen failed: " << dlerror() << "\n\n";
        return 1;
    }
    std::cout << "\n  Loaded TextSearch library successfully";

    // load the (extern "C") function that returns instance of ITextSearch*
    typedef ITextSearch*(*Create)();
    Create create = (Create)dlsym(handle, "CreateITextSearch");
    if(!create){
        std::cout << "\n  Failed to acquire ITextSearch create function\n\n";
        return 1;
    }
    std::cout << "\n  Acquired ITextSearch create function successfully";

    // initialize TextSearch, create() returns TextSearch* as specified in 
    // the function definition in ITextSearch.cpp file
    ITextSearch* txtSearcher = create();
    std::cout << "\n  Created TextSearch instance successfully\n";
    
    
    
    // the following is the code for FileManager load and creation
    
    void* handle2 = dlopen("libFileManager.so", RTLD_LAZY);
    if(!handle2){
        std::cout <<  "\n  dlopen failed: " << dlerror() << "\n\n";
        return 1;
    }
    std::cout << "\n  Loaded FileManager library successfully";
    
    // load the (extern "C") function that returns instance of IFileManager*
    typedef IFileManager*(*CreateFm)();
    CreateFm createFm = (CreateFm)dlsym(handle2, "CreateFileMgrInstance");
    if(!createFm){
        std::cout << "\n  Failed to acquire FileManager create function\n\n";
        return 1;
    }
    std::cout << "\n  Acquired FileManager create function successfully";

    IFileManager* fmgr = createFm();
    std::cout << "\n  Created FileManager instance successfully";
    fmgr->ConfigureSearcher(txtSearcher);
    std::cout << "\n  Passed TextSearch instance to FileManager instance successfully";
    fmgr->SetWorkingDirectory(wPath, Patterns, incSubs);
    fmgr->SetSearchString(toSearch);
    fmgr->PerformOperations();
    
    std::thread t(&ThreadProc, txtSearcher);
    
    txtSearcher->PutFile("quit", "quit");
    
    t.join();
    
    // freeing resources
    delete txtSearcher;
    delete fmgr;
    
    return 0;
}

