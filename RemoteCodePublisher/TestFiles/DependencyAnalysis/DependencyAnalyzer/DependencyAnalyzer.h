#pragma once
/////////////////////////////////////////////////////////////////////
// DependencyAnalyzer.h - Dependency Analyzer package              //
// Ver 1.0                                                         //
// Application: Project #2 - Dependency Analysis - CSE-687         //
// Platform:    Dell Inspiron 5520, Win 10, Visual Studio 2015     //
// Author:      Ammar Salman, EECS, Syracuse University            //
//              (313) 788-4694  hoplite.90@hotmail.com             //
/////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package is analyzes dependencies based on the extracted types
* held in the TypeTable. The process is simple and easy. It takes a
* lot of time to process.
*
* Public Interface:
* DependencyAnalyzer da;
* da.Analyze(argc, argv);
* cout << da.ShowDependencies();
* auto db = da.getDb();
* auto tt = da.getTT();
*
*
* Required Files:
* ---------------
*   - DependencyAnalyzer.h, TypeTable.h, NoSqlDb.h
*
* Build Process:
* --------------
*   devenv DependencyAnalysis.sln /debug rebuild
*
* Maintenance History:
* --------------------
* Ver 1.0 : 6 March 2017
* - first release
*
*/

#include <unordered_map>
#include <vector>
#include <set>
#include <string>

#include "..\TypeTable\TypeTable.h"
#include "..\NoSQLDB\NoSqlDb\NoSqlDb.h"

namespace DependencyAnalysis {
	using namespace std;
	using namespace Databases;


	class DependencyAnalyzer {
	public:
		DependencyAnalyzer();
		~DependencyAnalyzer();
		void Analyze(int argc, char ** argv);
		string ShowDependencies();
		const TypeTable& getTT() const { return typeTable; }
		NoSQLDB<string> getDb() { return dependencyDB; }

	private:
		TypeTable typeTable;
		NoSQLDB<string> dependencyDB;
		unordered_map<string, vector<TypeValue>> definedTypes;
		unordered_map<string, set<string>> usedTypes;
		unordered_map<string, vector<string>> includedLibs;

		void processHeaderSrouceDeps(string fName);
		void processUsedTypeDeps(string fName);
		void AddFilesToDB();
	};

	template<typename T>
	const bool vectorContains(vector<T>& vec, T value) {
		for (auto val : vec) {
			if (val == value) return true;
		}
		return false;
	}

}