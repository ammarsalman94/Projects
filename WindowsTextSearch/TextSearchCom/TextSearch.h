#pragma once
///////////////////////////////////////////////////////////////////////
//  TextSearch.h  - Accepts files and search patterns to check       //
//  ver 1.0                                                          //
//                                                                   //
//  Language:     C++                                                //
//  Platform:     Dell Inspiron 5520, Windows 10 Professional        //
//  Application:  Destributed Objects (CSE 775) Project #1           //
//  Author:       Ammar Salman, Syracuse University                  //
//                (313) 788-4694, hoplite.90@hotmail.com             //
///////////////////////////////////////////////////////////////////////
/*
Component Operations:
=====================
Provides an implementation that accepts files and search patterns. Each 
file is checked to see whether or not it contained the search pattern
specified. The results are pushed into an outgoing queue.

*/
///////////////////////////////////////////////////////////////////////
//  Build Process                                                    //
///////////////////////////////////////////////////////////////////////
//  Required files:                                                  //
//    TextSearchCom_i.c, TextSearchCom_i.h, TextSearchCom.idl        //
//    TextSearch.h, TextSearch.cpp, TextSearchCom.h,                 //
//    TextSearchCom.cpp, Resource.h, TextSearchCom.def, xdlldata.c,  //
//    dllmain.cpp, stdafx.h, stdafx.cpp                              //
//    Cpp11-BlockingQueue.h, Cpp11-BlockingQueue.cpp                 //
//  Note:                                                            //
//    all of these are provided by ATL.                              //
//    The developer modifies TextSearchCom.idl, TextSearchCom.cpp,   //
//    and TextSearchCom.h                                            //
//    The BlockingQueue files are provided by the professor as help  //
//                                                                   //
//  Build process:                                                   //
//    - right click on TextSearchCom.idl and compile                 //
//    - right click on TextSearchCom and rebuild                     //
///////////////////////////////////////////////////////////////////////
/*
Maintenance History:
====================
ver 1.0 : 19 Mar 2017
- first release

*/

#include "resource.h"      

#include "TextSearchCom_i.h"
#include "Cpp11-BlockingQueue.h"
#include <thread>
#include <fstream>
#include <string>

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

using namespace ATL;


class ATL_NO_VTABLE CTextSearch :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CTextSearch, &CLSID_TextSearch>,
	public IDispatchImpl<ITextSearch, &IID_ITextSearch, &LIBID_TextSearchComLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
private:
	BlockingQueue<std::pair<BSTR, BSTR>> inQ;
	BlockingQueue<std::pair<std::pair<BSTR, BSTR>, VARIANT_BOOL>> outQ;

	VARIANT_BOOL SearchFile(BSTR fPath, BSTR toSearch);
	void threadProc();

public:
	CTextSearch();

DECLARE_REGISTRY_RESOURCEID(IDR_TEXTSEARCH)


BEGIN_COM_MAP(CTextSearch)
	COM_INTERFACE_ENTRY(ITextSearch)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:
	STDMETHOD(PutFile)(BSTR fPath, BSTR toSearch);
	STDMETHOD(GetResult)(BSTR* fPath, BSTR* toSearch, VARIANT_BOOL* contains);
};

OBJECT_ENTRY_AUTO(__uuidof(TextSearch), CTextSearch)


namespace TS_Util {

	////////////////////////////////////////////////////////////////////////////////////////////
	// The following code is from Stack Overflow. Author: WhozCraig.                          //
	// Link: http://stackoverflow.com/a/13726895/6871623                                      //
	////////////////////////////////////////////////////////////////////////////////////////////

	// convert a BSTR to a std::string. 
	inline std::string& BstrToStdString(const BSTR bstr, std::string& dst, int cp = CP_UTF8) {
		if (!bstr) {
			// define NULL functionality. I just clear the target.
			dst.clear();
			return dst;
		}

		// request content length in single-chars through a terminating
		//  nullchar in the BSTR. note: BSTR's support imbedded nullchars,
		//  so this will only convert through the first nullchar.
		int res = WideCharToMultiByte(cp, 0, bstr, -1, NULL, 0, NULL, NULL);
		if (res > 0) {
			dst.resize(res);
			WideCharToMultiByte(cp, 0, bstr, -1, &dst[0], res, NULL, NULL);
		}
		else {    // no content. clear target
			dst.clear();
		}
		return dst;
	}

	// conversion with temp.
	inline std::string BstrToStdString(BSTR bstr, int cp = CP_UTF8) {
		std::string str;
		BstrToStdString(bstr, str, cp);
		return str.substr(0, str.size() - 1);
	}
}