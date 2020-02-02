#pragma once
/////////////////////////////////////////////////////////////////////
// RegularExpression.h - Regular expression parser for NoSqlDB.h	 //
// Ver 1.3																												 //
// Application: Project #1 - No-SQL Database - CSE-687 						 //
// Platform:    Dell Inspiron 5520, Win 10, Visual Studio 2015	   //
// Author:      Ammar Salman, EECS, Syracuse University						 //
//              (313) 788-4694  hoplite.90@hotmail.com						 //
/////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package parses text commands and extracts the required operations
* from the text. Currently it only supports 'get' command but the 
* structure is made in a way that allows for easy addition of commands
* and operations. 
* 
* Example: get name='ammar' or name='odai' and category='c++'
*		This will read into the following operations and arguments:
*			name		 - 'ammar'	or
*			name		 - 'odai'		and
*			category - 'c++'
*		The database is designed to execute the ANDs before the ORs.
*
* Required Files:
* ---------------
*   - RegularExpression.h, RegularExpression.cpp
*
* Build Process:
* --------------
*   devenv NoSQLDB.sln /debug rebuild
*
* Maintenance History:
* --------------------
* Ver 1.3 : 31 Jan 2017
* - support for add command
* - used regex for extraction of operations
*
* Ver 1.2 : 30 Jan 2017
* - support for delete command
* - support for parenthesis: replaced boolean priorities with
*   integer priorities as now we have four (highest is lowest):
*			 1- AND inside parenthesis
*			 2- OR inside parenthesis
*			 3- AND outside parenthesis
*			 4- OR outside parenthesis
*
* Ver 1.0 : 29 Jan 2017
* - first release
*
* ToDo:
* -----
* Add support for (add) command
*
*/
#include <string>
#include <vector>

using namespace std;

class RegularExpression {
private:

	string QueryText;

	//extracted command from the query
	string cmd;
	// these three vectors hold what operations to do and what are their arguments
	vector<string> operations;
	vector<string> arguments;
	// For compound queries, priorities are listed as:
	// 1- AND inside parenthesis
	// 2- OR inside parenthesis
	// 3- AND outside parenthesis
	// 4- OR outside parenthesis
	vector<int> priorities;

	// accessory method
	bool find(vector<string> vec, string key);

public:
	// constructors
	RegularExpression();
	RegularExpression(string QueryText);

	// query processing methods
	void ProcessQuery();
	void ProcessQuery(string QueryText);

	// used for the database to return the extracted 
	// words from the query for further processing
	string GetCommand() { return cmd; }
	vector<string> GetOperations() { return operations; }
	vector<string> GetArguments() { return arguments; }
	vector<int> GetCompoundParameters(){ return priorities; }

};

