/////////////////////////////////////////////////////////////////////
// DependencyAnalyzer.cpp - Dependency Analyzer package            //
// Ver 1.0                                                         //
// Application: Project #2 - Dependency Analysis - CSE-687         //
// Platform:    Dell Inspiron 5520, Win 10, Visual Studio 2015     //
// Author:      Ammar Salman, EECS, Syracuse University            //
//              (313) 788-4694  hoplite.90@hotmail.com             //
/////////////////////////////////////////////////////////////////////

#include <vector>
#include <iostream>
#include <iomanip>

#include "DependencyAnalyzer.h"
#include "..\NoSQLDB\NoSqlDb\Element.h"

namespace DependencyAnalysis {
	using namespace std;

	DependencyAnalyzer::DependencyAnalyzer() {
	}


	DependencyAnalyzer::~DependencyAnalyzer() {
	}

	// -----< Analyze function >---------------------------------------------------------
	/* takes arguments information and analyzes everything */
	void DependencyAnalysis::DependencyAnalyzer::Analyze(int argc, char ** argv) {
		typeTable.ExtractDefinedTypes(argc, argv);
		typeTable.ExtractUsedTypes();
		definedTypes = typeTable.definedTypes();
		usedTypes = typeTable.usedTypes();
		includedLibs = typeTable.UsedLibraries();
		AddFilesToDB();

		for (auto it : definedTypes) {
			processHeaderSrouceDeps(it.first);
			processUsedTypeDeps(it.first);
		}
		
	}

	// -----< Show Dependencies function >------------------------------------------
	/* returns string showing dependency information */
	string DependencyAnalyzer::ShowDependencies() {
		ostringstream oss;
		for (auto file : dependencyDB.GetAllKeys()) {
			oss << "\n\n  " << setw(10) << left << "File:" << file;
			oss << "\n  " << setw(10) << left << "Depends On:";
			for (auto dep : dependencyDB.GetChildren(file)) {
				oss << "\n  " << setw(10) << "" << dep;
			}
		}
		return oss.str();
	}


	// -----< Process header/source dependencies function >------------------------
	/* checks if there exists header/source files that depend on each other to run*/
	void DependencyAnalyzer::processHeaderSrouceDeps(string fName) {
		for (auto it : definedTypes) {
			if (it.first == fName) continue;
			bool doBreak = false;
			for (auto tv1 : definedTypes[fName]) {
				for (auto tv2 : definedTypes[it.first]) {
					if (tv1.Type() == "function" && tv1.ParentNamespace() != "class") continue;
					if (tv1 == tv2 && tv1.QualifiedName().size() > 1 && tv1.Type() != "namespace") {
						dependencyDB.AddRelation(fName, it.first);
						dependencyDB.AddRelation(it.first, fName);
						doBreak = true;
						break;
					}
				}
				if (doBreak) break;
			}
		}
	}

	// -----< Process used types dependencies >---------------------------------------------
	/* checks definedTypes for each file against usedTypes of every other file. If there is
	 * dependency, add it and move on */
	void DependencyAnalyzer::processUsedTypeDeps(string fName) {
		for (auto it : usedTypes) {
			if (it.first == fName) continue;
			bool doBreak = false;
			for (auto name : it.second) {
				for (auto tv : definedTypes[fName]) {
					if (name == tv.Name() && DependencyAnalysis::vectorContains(includedLibs[it.first], fName)) {
						dependencyDB.AddRelation(it.first, fName);
						doBreak = true;
						break;
					}
				}
				if (doBreak) break;
			}
		}
	}

	// -----< Add files to database function >-------------------------------------------
	/* after the type extraction, get all files to analyze their dependencies */
	void DependencyAnalyzer::AddFilesToDB() {
		for (auto iter : definedTypes) {
			Element<string> el;
			el.SetKey(iter.first);
			//string str = iter.first.substr(iter.first.find_last_of('\\' + 1)
			//el.SetName();
			dependencyDB.AddEntry(el.GetKey(), el);
		}
		for (auto iter : usedTypes) {
			Element<string> el;
			el.SetKey(iter.first);
			//string str = iter.first.substr(iter.first.find_last_of('\\' + 1)
			//el.SetName();
			dependencyDB.AddEntry(el.GetKey(), el);
		}
	}
	
}

#ifdef TEST_DEPANALYSIS

int main(int argc, char ** argv) {
	using namespace DependencyAnalysis;
	using namespace std;
	try {
		DependencyAnalyzer da;
		da.Analyze(argc, argv);
		cout << da.ShowDependencies();
	}
	catch (exception& ex) {
		cout << "\n  Dependency Analyzer failed. Details: " << ex.what();
	}
	return 0;
}

#endif
