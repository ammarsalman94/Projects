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


#include "ITextSearch.h"
#include "Cpp11-BlockingQueue.h"
#include <string>
#include <fstream>
#include <regex>
#include <thread>


class TextSearch : public ITextSearch{
private:
    BlockingQueue<Pair> inQ;
    BlockingQueue<Record> outQ;
    bool SearchFile(std::string fPath, std::string toSearch);
    void *ThreadProc(void);
    std::thread t;
    
    bool SearchODF(std::string fPath, std::string toSearch);
    
public:
    TextSearch(void);
    virtual ~TextSearch(void);
    virtual void PutFile(std::string, std::string) override;
    virtual Record GetRecord() override;
};

// -----< Constructor >---------------------------------------------
/* Initializes the worker thread on the current instance resources*/
TextSearch::TextSearch(){
    t = std::thread(&TextSearch::ThreadProc, this);
}

// -----< Destructor >----------------------------------------------
/* Enqueues quit message and waits for the worker thread to quit*/
TextSearch::~TextSearch(){
    if(t.joinable()){
        PutFile("quit", "quit");
        t.join();
    }
}

// -----< Put File >------------------------------------------------
/* Enqueues file path and string to search for */
void TextSearch::PutFile(std::string fPath, std::string toSearch){
    Pair in(fPath, toSearch);
    inQ.enQ(in);
}

// -----< Get Record >----------------------------------------------
/* Returns Record for the oldest processed file*/
Record TextSearch::GetRecord(){
    return outQ.deQ();
}

// -----< Search File >---------------------------------------------
/* Searches a file for some string and returns boolean */
bool TextSearch::SearchFile(std::string fPath, std::string toSearch){
    size_t dotIndex = fPath.find_last_of('.');
    std::string extension = fPath.substr(dotIndex
            , fPath.size() - dotIndex);
    if(extension == ".odf") ;
    std::ifstream ifs(fPath);
    std::string s = "(.*)" + toSearch + "(.*)|(.*)" 
            + toSearch + "|" + toSearch + "(.*)";
    std::regex rgx(s, std::regex_constants::ECMAScript 
                    | std::regex_constants::icase);
    std::string line;
    while (ifs.good()) {
            std::getline(ifs, line);
            if (std::regex_match(line, rgx)) return true;
    }
    return false;
}

// -----< Search ODF >------------------------------------------------
/* Takes ODF file path and string to search for. Extracts the ODF
 * into temp directory and gets its content.xml which holds the 
 * inner text, then it searches it and returns result */
bool TextSearch::SearchODF(std::string fPath, std::string toSearch){
    // TODO: extract ODF file and call SearchFile on its content.xml
    // std::string content = "<extracted directory>/content.xml";
    // return SearchFile(content, toSearch);
    
    // nothing is implemented, just return false as we cant open it yet
    return false;
}

// -----< Thread Processing >----------------------------------------
/* Specifies the worker thread processing functionality */
void* TextSearch::ThreadProc(){
    while (true) {
        Pair info = inQ.deQ();
        if (info.first == "quit") {
            // put the quit message to the receiver
            // client so it can quit as well
            outQ.enQ(std::make_pair(info, false));
            break;
        }
        Record toOutQ(info, SearchFile(info.first, info.second));
        outQ.enQ(toOutQ);
    }
}


// -----< Create TextSearch >-----------------------------------------
/* This function was used for debugging purposes */
ITextSearch* ITextSearch::CreateTextSearch(){
    return new TextSearch;
}

// -----< Create TextSearch Instance >--------------------------------
/* This function is a C function with an unmangled name
 * that can be called by its name when the library
 * is loaded using `dlopen` function */
ITextSearch* CreateITextSearch(){
    return new TextSearch;
}
