///////////////////////////////////////////////////////////////////
// IFileManager.cpp : Defines FileManager class                  //
//                                                               //
// Platform:     Dell Inspiron 5520, Ubuntu 16.04, NetBeans 8.2  //
// Application:  Distributed Objects - CSE-775, Project 1 Sp17   //
// Author:       Ammar Salman, Syracuse University               //
//               (313) 788-4694, hoplite.90@hotmail.com          //
///////////////////////////////////////////////////////////////////
/*
 * This package defines FileManager class that inherits from inter-
 * face IFileManager and implements its methods. 
 * It accepts ITextSearch* and sends it the extracted file list.
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
 *  - FileSystem.h
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


#include "IFileManager.h"
#include "../TextSearch/ITextSearch.h"
#include "FileSystem.h"

#include <vector>
#include <string>
#include <sstream>

using namespace FileSystem;

class FileManager : public IFileManager{
private:
    ITextSearch* txtSearcher;
    std::string sString;
    std::vector<std::string> PatternList;
    std::vector<std::string> FilePaths;
    
    void ExtractPatterns(std::string Patterns);
    void SearchDirectories(std::string wPath, bool incSubDirs);
    
public:
    FileManager(){}
    virtual ~FileManager(){}
    virtual void ConfigureSearcher(ITextSearch*) override;
    virtual void SetWorkingDirectory(std::string, std::string, bool) override;
    virtual void SetSearchString(std::string) override;
    virtual void PerformOperations() override;
    virtual void Clear() override;
};

// -----< Configure Text Search Instance >--------------------
/* Takes ITextSearch* and sets its txtSearcher accordingly */
void FileManager::ConfigureSearcher(ITextSearch* txtSrch){
    txtSearcher = txtSrch;
}


// -----< Set Working Directory >-----------------------------
/* Takes working directory path, patterns string and boolean
 * indicating whether or not to include sub-directories*/
void FileManager::SetWorkingDirectory(std::string wPath, 
        std::string Patterns, bool incSubDirs){
    ExtractPatterns(Patterns);
    SearchDirectories(wPath, incSubDirs);
}

// -----< Set Search String >---------------------------------
/* Takes string that specifies what to look for inside files*/
void FileManager::SetSearchString(std::string sString){
    this->sString = sString;
}

// -----< Perform Operations >--------------------------------
/* Sends all obtained files to txtSearcher for it to search */
void FileManager::PerformOperations(){
    for(std::string file : FilePaths)
        txtSearcher->PutFile(file, sString);
}

// -----< Clear >---------------------------------------------
/* Clears sString, obtained lists & txtSearcher pointer */
void FileManager::Clear(){
    sString = "";
    PatternList.clear();
    FilePaths.clear();
    txtSearcher = nullptr;
}

// -----< Extract Patterns >----------------------------------
/* Takes string specifying multiple patterns and extracts
 * them into PatternList vector for further processing */
void FileManager::ExtractPatterns(std::string Patterns){
    std::istringstream iss(Patterns);
    while(iss.good()){
        std::string temp;
        iss >> temp;
        PatternList.push_back(temp);
    }
}

// -----< Search Directories >-----------------------------------
/* Takes path and boolean indicating whether or not to include
 * sub-directories in the search. If yes, it will recurse 
 * through all of the sub-directories and add files to FilePaths*/
void FileManager::SearchDirectories(std::string wPath, bool incSubDirs){
    if(wPath == "") return;
    std::vector<std::string> temp;
    for(std::string Pattern : PatternList){
        temp = Directory::getFiles(wPath, Pattern);
        for(size_t i = 0; i < temp.size(); i++)
            temp[i] = Path::fileSpec(wPath, temp[i]);
        FilePaths.reserve(temp.size());
        FilePaths.insert(FilePaths.end(), temp.begin(), temp.end());
    }
    if(!incSubDirs) return;
    temp = Directory::getDirectories(wPath);
    for(size_t i = 0; i < temp.size(); i++)
            temp[i] = Path::fileSpec(wPath, temp[i]);
    for(std::string file : temp)
        SearchDirectories(file, incSubDirs);
}


// -----< Create FileManager >---------------------------
/* This function was used for debugging purposes */
IFileManager* IFileManager::CreateFileManager(){
    return new FileManager;
}

// -----< Create FileManager Instance >------------------
/* This function is a C function with an unmangled name
 * that can be called by its name when the library
 * is loaded using `dlopen` function */
IFileManager* CreateFileMgrInstance(){
    return new FileManager;
}
