///////////////////////////////////////////////////////////////////////
//  TextSearch.cpp  - Implementation file for TextSearch.h component //
//  ver 1.0                                                          //
//                                                                   //
//  Language:     C++, COM/IDL                                       //
//  Platform:     Dell Inspiron 5520, Windows 10 Professional        //
//  Application:  Destributed Objects (CSE 775) Project #1           //
//  Author:       Ammar Salman, Syracuse University                  //
//                (313) 788-4694, hoplite.90@hotmail.com             //
///////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "TextSearch.h"

#include <regex>
#include <fstream>
#include <string>
#include <thread>

using namespace TS_Util;

// provides the default contructor for TextSearch component
CTextSearch::CTextSearch() {
	std::thread t(&CTextSearch::threadProc, this);
	t.detach();
}


// uses regular expression to find the string in the specified file
VARIANT_BOOL CTextSearch::SearchFile(BSTR fPath, BSTR toSearch) {
	std::ifstream ifs(fPath);
	std::string strfrombstr = BstrToStdString(toSearch);
	std::string s = "(.*)" + strfrombstr + "(.*)|(.*)" + strfrombstr + "|" + strfrombstr + "(.*)";
	std::regex rgx(s, std::regex_constants::ECMAScript | std::regex_constants::icase);
	std::string line;
	while (ifs.good()) {
		std::getline(ifs, line);
		if (std::regex_match(line, rgx)) return VARIANT_TRUE;
	}
	return VARIANT_FALSE;
}

// specifies the processing thread functionality
// it stops when a quit message is found
void CTextSearch::threadProc() {
	while (true) {
		std::pair<BSTR, BSTR> info = inQ.deQ();
		std::string first = BstrToStdString(info.first);
		if (first == "quit") {
			// put the quit message to the receiver client so it can quit as well
			outQ.enQ(std::make_pair(info, VARIANT_FALSE));
			break;
		}
		VARIANT_BOOL contained = SearchFile(info.first, info.second);
		std::pair<std::pair<BSTR, BSTR>, VARIANT_BOOL> toOutQ(info, contained);
		outQ.enQ(toOutQ);
	}
}



// enqueues file and search string into the incoming queue
STDMETHODIMP CTextSearch::PutFile(BSTR fPath, BSTR toSearch) {
	CComBSTR path(fPath);
	CComBSTR srch(toSearch);
	inQ.enQ(std::make_pair(path.Detach(), srch.Detach()));
	// free the current parameters in order to make it available
	// for future calls without problems 
	SysFreeString(fPath);
	SysFreeString(toSearch);
	return S_OK;
}

// dequeues result entry from the outgoing queue and returns its values to the client
STDMETHODIMP CTextSearch::GetResult(BSTR* fPath, BSTR* toSearch, VARIANT_BOOL* contains) {
	std::pair<std::pair<BSTR, BSTR>, VARIANT_BOOL> res = outQ.deQ();
	CComBSTR path(res.first.first);
	CComBSTR srch(res.first.second);
	*fPath = path.Detach();
	*toSearch = srch.Detach();
	*contains = res.second;
	SysFreeString(res.first.first);
	SysFreeString(res.first.second);
	return S_OK;
}
