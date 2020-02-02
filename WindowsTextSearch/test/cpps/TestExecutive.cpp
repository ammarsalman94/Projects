/////////////////////////////////////////////////////////////////////
// TestExecutive.cpp - Demonstrates project functionality          //
// Ver 1.0                                                         //
// Application: Project #2 - Dependency Analysis - CSE-687         //
// Platform:    Dell Inspiron 5520, Win 10, Visual Studio 2015     //
// Author:      Ammar Salman, EECS, Syracuse University            //
//              (313) 788-4694  hoplite.90@hotmail.com             //
/////////////////////////////////////////////////////////////////////


#ifdef TEST_EXECUTIVE_DA

#include "..\DependencyAnalysis\DependencyAnalyzer.h"
#include "..\StrongComponents\StrongComponent.h"
#include "..\NoSQLDB\NoSqlDb\NoSqlDb.h"
#include "..\TypeTable\TypeTable.h"
#include <iostream>
#include <string>

void ShowUsage() {
	std::cout << "\n                         USAGE DETAILS";
	std::cout << "\n =============================================================";
	std::cout << "\n  First argument should be directory";
	std::cout << "\n  XML results files arguments should be in the end";
	std::cout << "\n  For example: ..\\TestFiles2 *.h *.cpp -depResults results.xml -scResults scResults.xml";
	std::cout << "\n\n";
}

bool ProcessCommandLine(int *argc, char**argv, string* dR, string* scR) {
	if (*argc < 3) {
		std::cout << "\n  Not enough arguments";
		ShowUsage();
		return false;
	}
	int fOcc = 0;
	for (int i = 0; i < *argc; i++) {
		if (argv[i] == std::string("-depResults") && (i + 1 < *argc)) {
			if (fOcc == 0) fOcc = i;
			*dR = argv[++i];
		}
		if (argv[i] == std::string("-scResults") && (i + 1 < *argc)) {
			if (fOcc == 0) fOcc = i;
			*scR = argv[++i];
		}
	}
	if (fOcc != 0) *argc = fOcc;
	return true;
}

int main(int argc, char ** argv) {
	using namespace std;
	using namespace DependencyAnalysis;
	using Utilities::StrongComponent;
	string depResults = "dependencies.xml";
	string scResults = "strongComponents.xml";
	try {
		if(!ProcessCommandLine(&argc, argv, &depResults, &scResults)) return 1;
		cout << "\n                            Starting Dependency Analyzer";
		cout << "\n ==================================================================================";
		DependencyAnalyzer da;
		cout << "\n\n  Processing directory: " << argv[1] << "   ...  ";
		da.Analyze(argc, argv);
		cout << "\n\n\n                Type Table - Extracted Information";
		cout << "\n ===============================================================";
		auto tt = da.getTT();
		auto depDb = da.getDb();
		tt.Print();
		cout << "\n\n\n           Dependency Analyzer - Analyzed Dependencies";
		cout << "\n ===============================================================";
		cout << da.ShowDependencies();

		cout << "\n\n\n                       Strong Components";
		cout << "\n ===============================================================";
		cout << "\n\n  Analyzing strong components ... ";

		StrongComponent sc;
		sc.processNoSqlDb(depDb);
		sc.extractStrongComponents();
		auto scDb = sc.StrongestComponents_NOSQLDB();

		cout << "\n\n  All Strong Components:\n" << scDb.ToString();
		cout << "\n\n  Strongest Component: \n" << sc.StrongestComponent_El().ToString();

		cout << "\n\n\n                        Saving Results";
		cout << "\n ===============================================================";

		cout << "\n\n  Storing analysis results in \"" << depResults << "\"";
		depDb.StoreXML(depResults);
		cout << "\n  Storing strong components in \"" << scResults <<"\"";
		scDb.StoreXML(scResults);
		
		cout << "\n\n";
	}
	catch (...) {
		cout << "\n\n  Analysis failed.\n\n";
	}
	return 0;
}

#endif