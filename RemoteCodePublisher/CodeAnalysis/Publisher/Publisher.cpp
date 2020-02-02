/////////////////////////////////////////////////////////////////////
// Publisher.cpp - Provides implementation for Publisher.h         //
// Ver 1.0                                                         //
// Application: Project #3 - Code Publisher - CSE-687              //
// Platform:    Dell Inspiron 5520, Win 10, Visual Studio 2015     //
// Author:      Ammar Salman, EECS, Syracuse University            //
//              (313) 788-4694  hoplite.90@hotmail.com             //
/////////////////////////////////////////////////////////////////////

#include "Publisher.h"
#include "../../SharedProjects/FileSystem/FileSystem.h"
#include "../DependencyAnalysis/DependencyAnalyzer.h"
#include "../../SharedProjects/NoSQLDB/NoSqlDb/NoSqlDb.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <set>
#include <time.h>
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
			startupFolder = Directory::getCurrentDirectory();
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
					Directory::setCurrentDirectory(startupFolder);
					string temp(argv[i + 1]);
					if (!Directory::exists(temp)) 
						Directory::create(temp);
					Directory::setCurrentDirectory(temp);
					outputFolder = Directory::getCurrentDirectory();
				}
			}
			// restore working directory
			Directory::setCurrentDirectory(startupFolder);
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
    Directory::setCurrentDirectory(startupFolder);

		ifstream src;
		ofstream dst;
		
		for (string file : files) {
			// try opening the file
			src.open(("resources\\" + file), ios::binary);
			if (src.good()) {
				// open the destination file (or create it) and copy everything to it
				dst.open((resourcesPath + "\\" + file), ios::binary);
				dst << src.rdbuf();
				dst.close();
				src.close();
			}
			// if any of the files was missing this function will inform the caller
			else {
				// allow for next file to be opened correctly
				src.close();
				toReturn = false;
			}
		}
		return toReturn;
	}

  // -----< Copy styles and scripts from dir to another >---------------------------
  bool Publisher::CopyStylesAndScripts(const string & from, const string & to) {
    // tells the caller whether or not all the files were copied successfully
    bool toReturn = true;

    if (!Directory::exists(from)) return false;

    vector<string> files{ "dark.css", "light.css", "mainpage.css", "scripts.js" };

    ifstream src;
    ofstream dst;

    for (string file : files) {
      // try opening the file
      src.open((from + "\\" + file), ios::binary);
      if (src.good()) {
        // open the destination file (or create it) and copy everything to it
        dst.open((to + "\\" + file), ios::binary);
        dst << src.rdbuf();
        dst.close();
        src.close();
      }
      // if any of the files was missing this function will inform the caller
      else {
        // allow for next file to be opened correctly
        src.close();
        toReturn = false;
      }
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
		vector<string> keys = db.GetAllKeys();
		sort(keys.begin(), keys.end());
		for (string key : keys) {
			if (showProgress) cout << "\r\n  Creating HTML for: " << key;
			ConvertToHTML(key);
		}
	}


	// -----< Creates MainPage that displays all generated webpages >--------------------
	void Publisher::CreateMainPage(string note) {
		ostringstream oss;
		oss << "<html>\r\n\t<head>";
		oss << "\r\n\t\t<link rel=\"stylesheet\" type=\"text/css\" href=\"resources/mainpage.css\"/>";
		oss << "\r\n\t</head>";
		oss << "\r\n\t<body>";
		oss << "\r\n\t\t<h1>Code Publisher</h1>";
		if (note != "") oss << "\r\n\t\t<h3>Note: " << note << "</h3>";
		oss << "\r\n\t\t<table>";
		oss << "\r\n\t\t\t<tr>";
		oss << "\r\n\t\t\t\t<th>Source File</th>";
		oss << "\r\n\t\t\t\t<th>Depends On</th>";
		oss << "\r\n\t\t\t</tr>";
		vector<string> keys = db.GetAllKeys();
		sort(keys.begin(), keys.end());
		for (string key : keys) {
			oss << "\r\n\t\t\t<tr>";
			oss << "\r\n\t\t\t\t<td><div><a href=\"" << Path::getName(key) << ".html\">" << Path::getName(key) << "</a></div></td>";
			oss << "\r\n\t\t\t\t<td>";
			for (string dep : db.GetChildren(key)) {
				oss << "\r\n\t\t\t\t\t<div><a href=\"" << Path::getName(dep) << ".html\">" << Path::getName(dep) << "</a></div>";
			}
			oss << "\r\n\t\t\t\t</td>";
			oss << "\r\n\t\t\t</tr>";
		}
    string mainpage = outputFolder + "\\MainPage.html";
    published_files_.push_back(mainpage);
		ofstream out(mainpage);
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

  // -----< get list of all published files >------------------------------------------
  const vector<string>& Publisher::PublishedFiles() const {
    return published_files_;
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
		headings << "\r\n</pre>\r\n  </body>\r\n</html>";
    string outfilename = outputFolder + "\\" + Path::getName(FilePath).append(".html");
    published_files_.push_back(outfilename);
		ofstream out(outfilename);
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
    headings << "<!DOCTYPE html>";
    headings << "\n<!------------------------------------------------------------------------";
    headings << "\n  " << Path::getName(FilePath) << ".html - Project 3 published file";
    time_t result = time(NULL);
    char str[26]; ctime_s(str, sizeof(str), &result);
    headings << "\n  Published: " << str;
    headings << "\n  Ammar Salman, CSE687 - Object Oriented Design, Spring 2017";
    headings << "\n ------------------------------------------------------------------------->\n";
		headings << "<html>";
		headings << "\r\n  <head>";
		headings << "\r\n     <link rel=\"stylesheet\" type=\"text/css\" href=\"resources/light.css\"/>";
		headings << "\r\n     <script src=\"resources/scripts.js\"></script>";
		headings << "\r\n     <button class=\"Fixed\" onclick=\"toggleStyle(this)\">Dark Theme</button>";
		headings << "\r\n  </head>";
		headings << "\r\n  <body>";
		headings << "\r\n   <h3>" << Path::getName(FilePath) << "</h3>";
		headings << "\r\n   <hr/>";
		headings << "\r\n   <div class = \"indent\">";
		headings << "\r\n     <h4>Dependencies:</h4>";
		for (auto dep : db.GetChildren(FilePath))
			headings << "\r\n     <div><a href=\"" << Path::getName(dep) << ".html\">" << Path::getName(dep) << "</a></div>";
		headings << "\r\n   </div>";
		headings << "\r\n   <hr/>";
		headings << "\r\n<pre>";
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
		cout << "\r\n                                Starting Code Publisher";
		cout << "\r\n =======================================================================================";
		CodePublisher::Publisher p;

		if (!p.ProcessCommandLineArgs(argc, argv)) {
			cout << "\r\n\r\n  Wrong command line args passed .. terminating";
			return 1;
		}
		cout << "\r\n  Output directory: " << p.OutputDirectory();

		cout << "\r\n\r\n  Performing dependency analysis on the given files";
		if (!p.PerformDependencyAnalysis()) {
			cout << "\r\n\r\n  Failed to perform dependency analysis on the given files";
			return 1;
		}

		cout << "\r\n  Copying styles and scripts to the output directory";
		if (!p.CopyStylesAndScripts())
			cout << "\r\n  Some (or all) of the styles/scripts were not copied succesffuly\r\n";

		cout << "\r\n  Creating MainPage";
		p.CreateMainPage("Only analyzed files directly related to Publisher. The other dependencies were not analyzed");

		cout << "\r\n  Creating HTML files for the given source files";
		p.PrintProgess();
		p.ProcessFiles();

		cout << "\r\n\r\n  Opening the generated MainPage.html";
		p.OpenMainPage();
		cout << "\r\n";
	}
	catch (...) {
		cout << "\r\n\r\n  Something went wrong ... terminating";
		return 1;
	}
	return 0;
}

#endif