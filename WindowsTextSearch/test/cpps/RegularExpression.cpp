#include "RegularExpression.h"
/////////////////////////////////////////////////////////////////////
// RegularExpression.cpp - Source code for RegularExpression.h		 //
// Ver 1.0																												 //
// Application: Project #1 - No-SQL Database - CSE-687 						 //
// Platform:    Dell Inspiron 5520, Win 10, Visual Studio 2015	   //
// Author:      Ammar Salman, EECS, Syracuse University						 //
//              (313) 788-4694  hoplite.90@hotmail.com						 //
/////////////////////////////////////////////////////////////////////

#include <string>
#include <algorithm>
#include <regex>
#include "../StrHelper.h"
#include <iostream>
using namespace std;


RegularExpression::RegularExpression() {
}

RegularExpression::RegularExpression(string QueryText) {
	this->QueryText = trim(QueryText);
	ProcessQuery();
}


/// <summary>Extracts commands, operations 
/// and arguments from the set query text</summary>
void RegularExpression::ProcessQuery() {
	transform(QueryText.begin(), QueryText.end(), QueryText.begin(), ::tolower);
	string temp = trim(QueryText.substr(0, QueryText.find_first_of(' ')));
	cmd = temp;

	regex rgx("\\bkey\\b|\\bname\\b|\\bcategory\\b|\\bdescription\\b|\\btimedate\\b|\\bchildren\\b|\\bdata");
	for (sregex_iterator it(QueryText.begin(), QueryText.end(), rgx), it_end; it != it_end; ++it)
		operations.push_back((*it)[0]);
	rgx = regex("'(.*?)'");
	for (sregex_iterator it(QueryText.begin(), QueryText.end(), rgx), it_end; it != it_end; ++it) {
		temp = (*it)[0].str();
		temp = temp.substr(temp.find_first_of('\'') + 1, temp.find_last_of('\'') - temp.find_first_of('\'') - 1);
		arguments.push_back(temp);

	}
	string t1, t2, t3, t4;
	size_t index1, index2;
	index1 = QueryText.find_first_of('(');
	index2 = QueryText.find_last_of(')');
	if (index1 > QueryText.size() || index2 > QueryText.size()) {
		index1 = 0;
		index2 = QueryText.size() - 1;
	}
	rgx = regex("\\band\\b");
	regex_replace(back_inserter(t1), QueryText.begin() + index1
		, QueryText.begin() + index2, rgx, "1");

	t1 = QueryText.replace(index1, index2, t1);

	rgx = regex("\\bor\\b");
	regex_replace(back_inserter(t2), t1.begin(), t1.end(), rgx, "2");
	rgx = regex("\\band\\b");
	regex_replace(back_inserter(t3), t2.begin(), t2.end(), rgx, "3");
	rgx = regex("\\bor\\b");
	regex_replace(back_inserter(t4), t3.begin(), t3.end(), rgx, "4");

	rgx = regex("\\b1\\b|\\b2\\b|\\b3\\b|\\b4\\b");
	for (sregex_iterator it(t4.begin(), t4.end(), rgx), it_end; it != it_end; ++it)
		priorities.push_back(stoi((*it)[0]));
}

/// <summary>Extracts commands, operations 
/// and arguments from given query text</summary>
/// <param name="QueryText">Specifies new query text</param>
void RegularExpression::ProcessQuery(string QueryText) {
	this->QueryText = trim(QueryText);
	ProcessQuery();
}


bool RegularExpression::find(vector<string> vec, string key) {
	for (size_t i = 0; i < vec.size(); i++)
		if (vec[i].compare(key) == 0) return true;
	return false;
}


#ifdef TEST_REGULAR_EXPRESSION
/// <summary>
/// Test stub for Regular Expression package.
/// </summary>
int main() {
	RegularExpression re("get name = 'ammar' and category='c++' or name='odai'");
	vector<string> v1 = re.GetOperations();
	vector<string> v2 = re.GetArguments();
	vector<int> v3 = re.GetCompoundParameters();
	int size = v1.size();
	for (int i = 0; i < size; i++) {
		cout << "\ncommand: " << v1[i] << "='" << v2[i] << "' ";
		if (i == v3.size()) continue;
		else {
			if (v3[i]) cout << "and";
			else cout << "or";
		}
	}
	cout << "\n\n";
	return 0;
}

#endif