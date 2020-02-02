#pragma once
/////////////////////////////////////////////////////////////////////
// Publisher.h - Produces webpages from source code files          //
// Ver 1.0                                                         //
// Application: Project #3 - Code Publisher - CSE-687              //
// Platform:    Dell Inspiron 5520, Win 10, Visual Studio 2015     //
// Author:      Ammar Salman, EECS, Syracuse University            //
//              (313) 788-4694  hoplite.90@hotmail.com             //
/////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package creates webpages based on the output of dependency 
* analysis when performed on a set of source code files.
*
* Public Interface:
* Publisher p;
* p.ProcessCommandLineArgs(argc, argv);
* p.CopyStylesAndScripts();
* p.PerformDependencyAnalysis(); // or if already processed use : p.SetAnalysisDB(database);
* p.PrintProgress();
* p.ProcessFiles();
* p.CreateMainPage();
* p.OpenMainPage();
* string dir = p.OutputDirectory();
*
* Required Files:
* ---------------
*   - Publisher.h, Publisher.cpp, DependencyAnalyzer.h, NoSqlDb.h, Tokenizer.h, FileSystem.h
*
* Build Process:
* --------------
*   devenv Publisher.sln /debug rebuild
*
* Maintenance History:
* --------------------
* Ver 1.0 : 30 March 2017
* - first release
*
*/


#include "..\DependencyAnalysis\CodeAnalyzer\Tokenizer\Tokenizer.h"
#include "..\DependencyAnalysis\NoSQLDB\NoSqlDb\NoSqlDb.h"
#include <string>
#include <vector>


using namespace std;
namespace CodePublisher {
	class Publisher {
	public:
		Publisher();
		~Publisher();
		bool ProcessCommandLineArgs(int argc, char** argv);
		bool CopyStylesAndScripts();
		bool PerformDependencyAnalysis();
		void SetAnalysisDB(Databases::NoSQLDB<string>& database);
		void ProcessFiles();
		void CreateMainPage();
		void OpenMainPage();
		const string& OutputDirectory() const;

		void PrintProgess(bool print=true);

	private:
		// data
		Scanner::Toker toker;
		vector<string> keyWords = { "Asm", "auto", "bool", "break", "case", "catch", "char", "class", "const_cast",
			"continue", "default", "delete", "do", "double", "else", "enum", "dynamic_cast", "extern", "false", "float",
			"for", "union", "unsigned", "using", "friend", "goto", "if", "inline", "int", "long", "mutable", "virtual",
			"namespace", "new", "operator", "private", "protected", "public", "register", "void", "reinterpret_cast",
			"return", "short", "signed", "sizeof", "static", "static_cast", "volatile", "struct", "switch", "template",
			"this", "throw", "true", "try", "typedef", "typeid", "unsigned", "wchar_t", "while", "typename" };
		int argc;
		char** argv;
		string outputFolder;
		Databases::NoSQLDB<string> db;

		bool showProgress;

		// functions 
		void ConvertToHTML(string& FilePath);
		string PrepareHeading(string& FilePath);
		void ProcessToken(string& tok);
		void FilterHTMLChars(string& tok);
		string FinalizeLines(string& tok, int& counter);

		// accessory function 
		template<typename T> bool contains(vector<T>& vec, T& data);
	};

	// -----< return true/false indicating whether or not vector contains some data >-----------
	template<typename T>
	inline bool Publisher::contains(vector<T>& vec, T& data) {
		for (T vItem : vec) if (vItem == data) return true;
		return false;
	}

}