/////////////////////////////////////////////////////////////////////
// Publisher.cpp - Provides implementation for Publisher.h         //
// Ver 1.0                                                         //
// Application: Project #3 - Code Publisher - CSE-687              //
// Platform:    Dell Inspiron 5520, Win 10, Visual Studio 2015     //
// Author:      Ammar Salman, EECS, Syracuse University            //
//              (313) 788-4694  hoplite.90@hotmail.com             //
/////////////////////////////////////////////////////////////////////

#include "Publisher.h"
#include "..\DependencyAnalysis\CodeAnalyzer\FileSystem\FileSystem.h"
#include "..\DependencyAnalysis\DependencyAnalysis\DependencyAnalyzer.h"
#include "..\DependencyAnalysis\NoSQLDB\NoSqlDb\NoSqlDb.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>
#include <iomanip>
#include <vector>
#include <Windows.h>


using namespace std;
using namespace FileSystem;


namespace CodePublisher {

	// -----< Constructors >-------------------------------------------------------------
	Publisher::Publisher() {
		// sets output directory to: {currentdirectory}\\webpages
		outputFolder = Directory::getCurrentDirectory()+"\\webpages\\";
	}

	Publisher::~Publisher() {
		// no need to delete argv because main will delete it when main ends
	}


	// -----< Processes Command Line Arguments >-----------------------------------------
	bool Publisher::ProcessCommandLineArgs(int argc, char ** argv) {
		if (argc < 2) return false;
		try {
			// save working directory
			string oldDir = Directory::getCurrentDirectory();
			this->argc = argc;
			this->argv = argv;
			string wDir(argv[1]);
			Directory::setCurrentDirectory(wDir);
			wDir = Directory::getCurrentDirectory();
			if (!Directory::exists(wDir)) return false;
			bool includeSubDirectories = false;
			for (int i = 2; i < argc; i++) {
				string temp(argv[i]);
				if (temp == "-output") {
					Directory::setCurrentDirectory(oldDir);
					string temp(argv[i + 1]);
					if (!Directory::exists(temp)) 
						Directory::create(temp);
					Directory::setCurrentDirectory(temp);
					outputFolder = Directory::getCurrentDirectory();
				}
			}
			// restore working directory
			Directory::setCurrentDirectory(oldDir);
			return true;
		}
		catch (...) {
			return false;
		}
	}

	// -----< Copies CSS and JS files to output directory resources >--------------------
	bool Publisher::CopyStylesAndScripts() {
		// tells the caller whether or not all the files were copied successfully
		bool toReturn = true;

		string resourcesPath = outputFolder + "\\resources";
		if (!Directory::exists(resourcesPath))
			Directory::create(resourcesPath);

		vector<string> files{ "dark.css", "light.css", "mainpage.css", "scripts.js" };

		ifstream src;
		ofstream dst;
		
		// save current directory because it will change after copying the first file
		string oldDir = Directory::getCurrentDirectory();
		for (string file : files) {
			// set current directory to the stored one to ensure the copy works
			Directory::setCurrentDirectory(oldDir);
			
			// try opening the file
			src.open("resources\\" + file, ios::binary);
			if (src.good()) {
				// open the destination file (or create it) and copy everything to it
				dst.open(resourcesPath + file, ios::binary);
				dst << src.rdbuf();
			}
			// if any of the files was missing this function will inform the caller
			else 
				toReturn = false;
		}
		return toReturn;
	}

	// -----< Performs dependency analysis on the given files >--------------------------
	bool Publisher::PerformDependencyAnalysis() {
		if (argc<2) return false;
		DependencyAnalysis::DependencyAnalyzer da;
		da.Analyze(argc, argv);
		db = da.getDb();
		return true;
	}

	// -----< Sets inner db to output of external dep analyzer >-------------------------
	void Publisher::SetAnalysisDB(Databases::NoSQLDB<string>& database) {
		db = database;
	}

	// -----< Processes the dep analysis results DB >------------------------------------
	void Publisher::ProcessFiles() {
		for (string key : db.GetAllKeys()) {
			if (showProgress) cout << "\n  Creating HTML for: " << key;
			ConvertToHTML(key);
		}
	}


	// -----< Creates MainPage that displays all generated webpages >--------------------
	void Publisher::CreateMainPage() {
		ostringstream oss;
		oss << "<html>\n\t<head>";
		oss << "\n\t\t<link rel=\"stylesheet\" type=\"text/css\" href=\"resources/mainpage.css\"/>";
		oss << "\n\t</head>";
		oss << "\n\t<body>";
		oss << "\n\t\t<h1>Code Publisher</h1>";
		oss << "\n\t\t<table>";
		oss << "\n\t\t\t<tr>";
		oss << "\n\t\t\t\t<th>Source File</th>";
		oss << "\n\t\t\t\t<th>Depends On</th>";
		oss << "\n\t\t\t</tr>";
		for (string key : db.GetAllKeys()) {
			oss << "\n\t\t\t<tr>";
			oss << "\n\t\t\t\t<td><div><a href=\"" << Path::getName(key) << ".html\">" << Path::getName(key) << "</a></div></td>";
			oss << "\n\t\t\t\t<td>";
			for (string dep : db.GetChildren(key)) {
				oss << "\n\t\t\t\t\t<div><a href=\"" << Path::getName(dep) << ".html\">" << Path::getName(dep) << "</a></div>";
			}
			oss << "\n\t\t\t\t</td>";
			oss << "\n\t\t\t</tr>";
		}
		ofstream out(outputFolder + "\\MainPage.html");
		out << oss.str();
		out.close();
	}

	// -----< launches MainPage with default browser >-----------------------------------
	void Publisher::OpenMainPage() {
		string path = outputFolder + "\\MainPage.html";
		wstring sPath(path.begin(), path.end());
		ShellExecute(NULL, L"open", sPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
		
		//system(("start \"\" \"" + path + "\"").c_str());  // <--- this also works
	}

	// -----< returns output directory >-------------------------------------------------
	const string & Publisher::OutputDirectory() const {
		return outputFolder;
	}

	// -----< set whether or not to print progress while processing ---------------------
	void Publisher::PrintProgess(bool print) {
		showProgress = print;
	}


	// -----< Creates HTML file for a source file given its path >-----------------------
	void Publisher::ConvertToHTML(std::string& FilePath) {
		ostringstream oss, headings;
		headings << PrepareHeading(FilePath);
		
		ifstream in(FilePath);
		//configure tokenizer
		toker.returnComments(); 
		toker.returnSpaces(true);  // this functionality was added to the original tokenizer
		toker.attach(&in);

		//skip first token (junk because of returnSpaces functionality)
		toker.getTok();

		int counter = 0;
		while (in.good()) {
			string tok = toker.getTok();
			ProcessToken(tok);
			oss << tok;
		}
		headings << FinalizeLines(oss.str(), counter);
		headings << "\n</pre>\n  </body>\n</html>";
		ofstream out((outputFolder+"\\"+Path::getName(FilePath).append(".html")));
		out << headings.str();
		out.close();
	}

	// -----< Processes token to add styles >----------------------------------------------
	void Publisher::ProcessToken(string & tok) {
		FilterHTMLChars(tok);
		if (contains(keyWords, tok)) tok = "<span class=\"key\">" + tok + "</span>";
		if (tok[0] == '\"' || tok[0] == '\'') tok = "<span class=\"str\">" + tok + "</span>";
		if (tok.size() > 2 && tok[0] == '/' && (tok[1] == '/' || tok[1] == '*')) {
			if (tok[1] == '*')
				tok = "{MARKED}<span class=\"comment\">" + tok + "</span></span>";
			else
				tok = "<span class=\"comment\">" + tok + "</span>";
		}
		if (tok == "#") {
			tok += toker.getTok();
			tok = "<span class=\"directive\">" + tok + "</span>";
			tok += toker.getTok();
			string included = toker.getTok();
			if (included == "<") {
				FilterHTMLChars(included);
				string last = toker.getTok();
				while (last != ">") {
					included += last;
					last = toker.getTok();
				}
				FilterHTMLChars(last);
				included += last;
				tok += "<span class=\"str\">" + included + "</span>";
			}
			else if (included[0] == '\"') tok += "<span class=\"str\">" + included + "</span>";
			else tok += included;
		}
		if (tok == "{")
			tok = "{MARKED}" + tok;
		if (tok == "}")
			tok += "</span>";
	}

	// -----< Prepares HTML elements before the source code >--------------------------------
	string Publisher::PrepareHeading(string& FilePath) {
		ostringstream headings;
		headings << "<html>";
		headings << "\n  <head>";
		headings << "\n     <link rel=\"stylesheet\" type=\"text/css\" href=\"resources/light.css\"/>";
		headings << "\n     <script src=\"resources/scripts.js\"></script>";
		headings << "\n     <button class=\"Fixed\" onclick=\"toggleStyle(this)\">Dark Theme</button>";
		headings << "\n  </head>";
		headings << "\n  <body>";
		headings << "\n   <h3>" << Path::getName(FilePath) << "</h3>";
		headings << "\n   <hr/>";
		headings << "\n   <div class = \"indent\">";
		headings << "\n     <h4>Dependencies:</h4>";
		for (auto dep : db.GetChildren(FilePath))
			headings << "\n     <div><a href=\"" << Path::getName(dep) << ".html\">" << Path::getName(dep) << "</a></div>";
		headings << "\n   </div>";
		headings << "\n   <hr/>";
		headings << "\n<pre>";
		return headings.str();
	}

	
	// -----< Converts the chars '<' and '>' to their string representations >------------
	void Publisher::FilterHTMLChars(string& tok) {
		size_t index;
		while ((index = tok.find('<')) != std::string::npos) {
			tok.replace(index, 1, "&lt");
		}
		while ((index = tok.find('>')) != std::string::npos) {
			tok.replace(index, 1, "&gt");
		}
	}

	// -----< Adds numbering and min/max elements >--------------------------------------
	string Publisher::FinalizeLines(string& tok, int& counter) {
		istringstream iss(tok);
		ostringstream oss;
		while (iss.good()) {
			string temp;
			getline(iss, temp);
			size_t index = 0;
			bool isMarked = false;
			while ((index = temp.find("{MARKED}", index)) != string::npos) {
				if (index > 0 && temp[index - 1] == '\"') {
					index++;
					continue;
				}
				string replacement = "<span id=\"sp" + to_string(counter) + "\">";
				temp.replace(index, 8, replacement);
				temp = "<span class=\"minmax\" onclick=\"toggleminmax(this, 'sp" + to_string(counter) + "')\"" +
					" onmouseover=\"highlight('sp" + to_string(counter) + "')\" onmouseout=\"removeHighlight('sp" + 
					to_string(counter) + "')\">-</span>" + temp;
				oss << "<span class=\"number\">  " << setw(6) << left << ++counter << "</span>" << temp << endl;
				isMarked = true;
				index++;
			}
			if (!isMarked)
				oss << "<span class=\"number\">  " << setw(8) << left << ++counter << "</span>" << temp << endl;
		}
		return oss.str();
	}

}

#ifdef TEST_PUBLISHER

int main(int argc, char** argv) {
	try {
		cout << "\n                                Starting Code Publisher";
		cout << "\n =======================================================================================";
		CodePublisher::Publisher p;

		if (!p.ProcessCommandLineArgs(argc, argv)) {
			cout << "\n\n  Wrong command line args passed .. terminating";
			return 1;
		}
		cout << "\n  Output directory: " << p.OutputDirectory();

		cout << "\n\n  Performing dependency analysis on the given files";
		if (!p.PerformDependencyAnalysis()) {
			cout << "\n\n  Failed to perform dependency analysis on the given files";
			return 1;
		}

		cout << "\n  Copying styles and scripts to the output directory";
		if (!p.CopyStylesAndScripts())
			cout << "\n  Some (or all) of the styles/scripts were not copied succesffuly\n";

		cout << "\n  Creating MainPage";
		p.CreateMainPage();

		cout << "\n  Creating HTML files for the given source files";
		p.PrintProgess();
		p.ProcessFiles();

		cout << "\n\n  Opening the generated MainPage.html";
		p.OpenMainPage();
		cout << "\n";
	}
	catch (...) {
		cout << "\n\n  Something went wrong ... terminating";
		return 1;
	}
	return 0;
}

#endif