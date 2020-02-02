#pragma once
///////////////////////////////////////////////////////////////////////
//  FileManager.h  - Accepts path and filter patterns for file mngmt //
//  ver 1.0                                                          //
//                                                                   //
//  Language:     C++, COM/IDL                                       //
//  Platform:     Dell Inspiron 5520, Windows 10 Professional        //
//  Application:  Destributed Objects (CSE 775) Project #1           //
//  Author:       Ammar Salman, Syracuse University                  //
//                (313) 788-4694, hoplite.90@hotmail.com             //
///////////////////////////////////////////////////////////////////////
/*
Component Operations:
=====================
Provides an implementation that accepts working path and searches for all
files under that path (optionally including sub-directories) that match
the specified filter patterns. It also accepts TextSearch instance to
check the gathered files for containing specified search string

*/
///////////////////////////////////////////////////////////////////////
//  Build Process                                                    //
///////////////////////////////////////////////////////////////////////
//  Required files:                                                  //
//    FileMgr_i.c, FileMgr_i.h, FileMgr.idl, FileManager.h,          //
//    FileManager.cpp, Resource.h, FileMgr.def, xdlldata.c,          //
//    dllmain.cpp, stdafx.h, stdafx.cpp                              //
//    FileSystem.h, FileSystem.cpp                                   //
//  Note:                                                            //
//    all of these are provided by ATL.                              //
//    The developer modifies FileMgr.idl, FileManager.cpp,           //
//    and FileManager.h                                              //
//    The FileSystem package was provided by the professor as help   //
//                                                                   //
//  Build process:                                                   //
//    - right click on FileMgr.idl and compile                       //
//    - right click on FileMgr and rebuild                           //
///////////////////////////////////////////////////////////////////////
/*
Maintenance History:
====================
ver 1.0 : 19 Mar 2017
- first release

*/

#include "resource.h" 

#include "FileMgr_i.h"
#include <string>
#include <vector>
#include "..\TextSearchCom\TextSearchCom_i.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

using namespace ATL;


// CFileManager

class ATL_NO_VTABLE CFileManager :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CFileManager, &CLSID_FileManager>,
	public IDispatchImpl<IFileManager, &IID_IFileManager, &LIBID_FileMgrLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{

private:
	CComQIPtr<ITextSearch> txtSearcher;
	BSTR sString;
	std::string wPath;
	std::vector<std::string> Patterns;
	std::vector<std::string> FilePaths;

	void ProcessPatterns(std::string Patterns);
	void SearchFiles(std::string wPath, VARIANT_BOOL incSubDirs);

	VARIANT_BOOL isGood;

public:
	CFileManager() {
		wPath = ".";
		sString = L"No Specified String";
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FILEMANAGER)


BEGIN_COM_MAP(CFileManager)
	COM_INTERFACE_ENTRY(IFileManager)
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



	STDMETHOD(SetWorkingDirectory)(BSTR wPath, BSTR Pattern, VARIANT_BOOL incSubDirs);
	STDMETHOD(GetFile)(BSTR* file);
	STDMETHOD(good)(VARIANT_BOOL* isGood);
	STDMETHOD(ConfigureSearcher)(IDispatch* txtSrch);
	STDMETHOD(SetSearchString)(BSTR sString);
	STDMETHOD(PerformOperations)();
	STDMETHOD(TerminateSearcher)();
	STDMETHOD(Clear)();
};

OBJECT_ENTRY_AUTO(__uuidof(FileManager), CFileManager)



namespace FMgr_Util {

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