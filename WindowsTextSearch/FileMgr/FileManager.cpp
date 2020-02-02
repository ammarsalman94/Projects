///////////////////////////////////////////////////////////////////////
//  FileManager.cpp  - Implementation file for FileManager.h         //
//  ver 1.0                                                          //
//                                                                   //
//  Language:     C++, COM/IDL                                       //
//  Platform:     Dell Inspiron 5520, Windows 10 Professional        //
//  Application:  Destributed Objects (CSE 775) Project #1           //
//  Author:       Ammar Salman, Syracuse University                  //
//                (313) 788-4694, hoplite.90@hotmail.com             //
///////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FileManager.h"
#include "FileSystem.h"

#include <sstream>
#include <iostream>
#include "..\TextSearchCom\TextSearchCom_i.h"

using namespace FileSystem;
using namespace FMgr_Util;

// takes string specifying one or more patterns to set the filters to
void CFileManager::ProcessPatterns(std::string Patterns) {
	std::istringstream iss(Patterns);
	std::string temp;
	while (iss.good()) {
		iss >> temp;
		this->Patterns.push_back(temp);
	}
}

// optionally recursive function that searches a directory for files matching
// the specified pattern filters for files
void CFileManager::SearchFiles(std::string wPath, VARIANT_BOOL incSubDirs) {
	if (wPath == "") return;
	for (std::string pattern : Patterns) {
		std::vector<std::string> foundFiles = Directory::getFiles(wPath, pattern);
		for (size_t i = 0; i < foundFiles.size(); i++)
			foundFiles[i] = Path::fileSpec(wPath, foundFiles[i]);
		// push found files in the filepaths list
		FilePaths.reserve(foundFiles.size());
		FilePaths.insert(FilePaths.end(), foundFiles.begin(), foundFiles.end());
	}

	if (FilePaths.size() > 0) isGood = VARIANT_TRUE;

	// incSubDirs gives an option to search sub-directories or not
	if (incSubDirs == VARIANT_FALSE) return;

	std::vector<std::string> subDirs = Directory::getDirectories(wPath);
	for (size_t i = 0; i < subDirs.size(); i++) {
		if (subDirs[i] == "." || subDirs[i] == "..") continue;
		subDirs[i] = Path::fileSpec(wPath, subDirs[i]);
	}
	for (std::string directory : subDirs) {
		// current and parent directories should not be accounted for
		// only child directories count here
		if (directory == "." || directory == "..") continue;
		// recursivly get all files in all sub-directories
		SearchFiles(directory, incSubDirs);
	}
}

// wPath: the path to look for files in (e.g. C:\test)
// Pattern: multiple patterns to filter files (e.g. "*.h *.cpp *.txt" or "*.txt")
// incSubDirs: boolean indicating whether to look inside sub-directories or not
STDMETHODIMP CFileManager::SetWorkingDirectory(BSTR wPath, BSTR Pattern, VARIANT_BOOL incSubDirs) {
	this->wPath = BstrToStdString(wPath);
	ProcessPatterns(BstrToStdString(Pattern));
	SearchFiles(this->wPath, incSubDirs);
	SysFreeString(wPath);
	SysFreeString(Pattern);
	return S_OK;
}


// returns list of files found, one at a time
STDMETHODIMP CFileManager::GetFile(BSTR* file) {
	static size_t i = 0;
	if (i >= FilePaths.size()) {
		isGood = VARIANT_FALSE;
		return S_FALSE;
	}
	CComBSTR temp(FilePaths[i++].c_str());
	*file = temp.Detach();
	return S_OK;
}


// for returning a list of the found files under the specified path
STDMETHODIMP CFileManager::good(VARIANT_BOOL* isGood) {
	*isGood = this->isGood;
	return S_OK;
}


// accepts TextSearch instance to bind to. We used IDispatch
// because clients will see two ITextSearch types:
//		FileMgrLib::ITextSearch
//		TextSearchComLib::ITextSearch
// We know both of them are the same, but for clients they dont
// so we accept IDispatch and bind it to the member:
//		CComQIPtr<ITextSearch> txtSearcher;
// And it works because ITextSearch inherets from IDispatch
STDMETHODIMP CFileManager::ConfigureSearcher(IDispatch* txtSrch) {
	this->txtSearcher = txtSrch;
	return S_OK;
}


// set the string that we want to search for in the files
// found under the working directory
STDMETHODIMP CFileManager::SetSearchString(BSTR sString) {
	// make copy of the search string
	CComBSTR temp(sString);
	this->sString = temp.Detach();
	// free the parameter so we dont have leaked memory
	SysFreeString(sString);
	return S_OK;
}


// send all the found files under the specified path
// to the TextSearch instance so it could process it
STDMETHODIMP CFileManager::PerformOperations() {
	for (auto file : FilePaths) {
		// make copies of the filepath
		CComBSTR filePath(file.c_str());
		// make copy of the search string, this is necessary
		// because if we dont do it, the same pointer will 
		// be passed multiple times creating a mess
		CComBSTR search(sString);
		BSTR toSendPath = filePath.Detach();
		BSTR sSearch = search.Detach();
		//std::wcout << "\n  " << toSendPath << "   '" << sSearch << "'";
		txtSearcher->PutFile(toSendPath, sSearch);
	}
	return S_OK;
}

// TextSearch terminates its processing thread when 
// it receives quit message. We send it quit message here
STDMETHODIMP CFileManager::TerminateSearcher() {
	BSTR temp = SysAllocString(L"quit");
	txtSearcher->PutFile(temp, temp);
	return S_OK;
}


// clears all set data to make space for reuseability
STDMETHODIMP CFileManager::Clear() {
	FilePaths.clear();
	Patterns.clear();
	wPath = ".";
	sString = L"No Specified String";
	txtSearcher = nullptr;
	return S_OK;
}
