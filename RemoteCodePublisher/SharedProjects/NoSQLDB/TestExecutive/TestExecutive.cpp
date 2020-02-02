/////////////////////////////////////////////////////////////////////
// TestExecutive.cpp - Demonstration of NoSQLDB project						 //
// Ver 1.0																												 //
// Application: Project #1 - No-SQL Database - CSE-687 						 //
// Platform:    Dell Inspiron 5520, Win 10, Visual Studio 2015	   //
// Author:      Ammar Salman, EECS, Syracuse University						 //
//              (313) 788-4694  hoplite.90@hotmail.com						 //
/////////////////////////////////////////////////////////////////////
/** Package Operations:
* -------------------
* This package demonstrates meeting requirements of NoSQLDB
*
* Required Files:
* ---------------
*   - NoSqlDb.h, Element.h, StrHelper.h
*
* Build Process:
* --------------
*   devenv NoSQLDB.sln /debug rebuild
*
* Maintenance History:
* --------------------
* Ver 1.0 : 31 Jan 2017
* - first release*/

#include <string>
#include <vector>
#include <ostream>
#include <iostream>
#include <iomanip>
#include <Windows.h>

#include "..\NoSqlDb\Element.h"
#include "..\NoSqlDb\NoSqlDb.h"

#include "..\StrHelper.h"

using namespace std;

#ifdef TEST_TEXECUTIVE


void Title(string str, char delimeter = '=') {
	str = trim(str);
	ostringstream oss;
	oss << "\n  " << str << "\n ";
	int size = str.size() + 2;
	for (int i = 0; i < size; i++) oss << delimeter;
	oss << endl;
	cout << oss.str();
}

void DemonstrateElement() {
	Element<string> e1;
	e1.SetKey("key1");
	e1.SetName("element1");
	e1.SetCategory("C++");
	e1.SetDescription("example element");
	e1.SetData("i love c++");
	e1.AddChild("key2");
	e1.AddChild("key3");
	cout << endl;
	Title("Testing Element<string>", '-');
	cout << "\n  Printing element:";
	cout << "\n" << e1.ToString();
	cout << "\n\n  Printing element to XML:";
	cout << "\n" << e1.ToXMLString();

	vector<int> v{ 0,1,2,3,4,5,6,7,8,9 };
	Element<vector<int>> e2;
	e2.SetKey("key2");
	e2.SetName("element1");
	e2.SetCategory("C++");
	e2.SetDescription("example element");
	e2.SetData(v);
	e2.AddChild("key5");
	cout << endl;
	Title("Testing Element<vector<int>>", '-');
	cout << "\n  Printing element:";
	cout << "\n" << e2.ToString();
	cout << "\n\n  Printing element to XML:";
	cout << "\n" << e2.ToXMLString();
	cout << "\n\n";
}

Element<string> GenerateElement(string key) {
	Element<string> e1;
	e1.SetKey(key);
	e1.SetName("element");
	e1.SetCategory("C++");
	e1.SetDescription("example element");
	e1.SetData("i love c++");
	e1.AddChild("key6");
	e1.AddChild("key3");
	return e1;
}

void DemoAditionDeletion() {
	Title("Testing Addition/Deletion of Key/Value Pairs", '-');
	NoSQLDB<string> DB;
	cout << "\n  Adding first element to database";
	DB.AddEntry("key1", GenerateElement("key1"));
	cout << "\n  Adding second element to database";
	DB.AddEntry("key2", GenerateElement("key2"));
	cout << "\n  Database content:\n" << DB.ToString();
	cout << "\n\n  Deleting element with key1 from the database";
	DB.RemoveEntry("key1");
	cout << "\n  Database content:\n" << DB.ToString();
	cout << "\n\n";
}

void DemoEditingFunctions() {
	Title("Testing editing functions on value element", '-');
	NoSQLDB<string> DB;
	DB.AddEntry("key", GenerateElement("key"));
	cout << "\n  Contents of element with key = 'key':\n" << DB.GetElement("key").ToString();
	cout << "\n\n  Adding child relation 'key4' to 'key'";
	DB.AddRelation("key", "key4");
	cout << "\n  Removing child relation 'key3' from 'key'";
	DB.RemoveRelation("key", "key3");
	cout << "\n  Changing name to 'Jack Sparrow'";
	DB.ChangeName("key", "Jack Sparrow");
	cout << "\n  Changing category to 'Pirate'";
	DB.ChangeCategory("key", "Pirate");
	cout << "\n  Changing description to 'The best pirate ever'";
	DB.ChangeDescription("key", "The best pirate ever");
	cout << "\n\n  Contents of element with key = 'key':\n" << DB.GetElement("key").ToString();
	cout << "\n\n";
}

void DemoPersistance() {
	Title("Testing database persistance", '-');
	cout << "\n  Creating simple database";
	NoSQLDB<string> DB;
	DB.AddEntry("key1", GenerateElement("key1"));
	DB.AddEntry("key2", GenerateElement("key2"));
	cout << "\n  Database contents:\n" << DB.ToString();
	cout << "\n\n  Exporting from database to 'simpleDB.xml'";
	DB.StoreXML("simpleDB.xml");
	cout << "\n  Importing from 'simpleDB.xml' to database2";
	NoSQLDB<string> DB2;
	DB2.ImportXML("simpleDB.xml");
	cout << "\n  Database2 contents:\n" << DB2.ToString();
	cout << "\n\n";
}

void DemoAutoPersistance() {
	Title("Testing automatic database persistance", '-');
	NoSQLDB<string> DB;
	cout << "\n  Turning on time based automatic persistance every 2 seconds";
	DB.SetTimedSave(2, "db_autosave.xml");
	cout << "\n  Adding element to database";
	DB.AddEntry("key1", GenerateElement("key1"));
	cout << "\n  Database content:\n" << DB.ToString();
	cout << "\n  Sleeping for 2 seconds";
	Sleep(2000);
	cout << "\n  Turning off time based automatic persistance";
	DB.UnsetTimedSave();

	cout << "\n\n  Importing auto-saved XML to new database";
	NoSQLDB<string> DB2;
	DB2.ImportXML("db_autosave.xml");
	cout << "\n  Imported database content:\n" << DB2.ToString();
	cout << "\n\n  Turning on writes based automatic save (persist every write)";
	DB.SetWriteCounter(1, "db2_autosave.xml");
	cout << "\n  Adding element to database";
	DB.AddEntry("key2", GenerateElement("key2"));

	cout << "\n\n  Importing auto-saved XML to new database";
	DB2.ImportXML("db2_autosave.xml");
	cout << "\n  Imported database content:\n" << DB2.ToString();
	cout << "\n\n";
}

void DemoQueries(string dbPath) {
	Title("Testing different queries on attached XML",'-');
	NoSQLDB<string> DB;
	DB.ImportXML(dbPath);
	cout << "\n  Database content:\n" << DB.ToString();
	
	cout << "\n\n  Getting the value of key = 'Element_H' :";
	cout << "\n  Retreived element content:\n" << DB.GetElement("Element_H").ToString();

	cout << "\n\n  Getting the children of key = 'StrHelper_H' :";
	vector<string> vec = DB.GetChildren("StrHelper_H");
	int size = vec.size();
	cout << "\n  Retreived children: ";
	for (int i = 0; i < size; i++) cout << "\n\t\t" << vec[i];

	cout << "\n\n  Getting keys matching pattern (invalid pattern specified, return all keys): ";
	vec = DB.Keys_With_Pattern("");
	cout << "\n  Retreived keys: ";
	size = vec.size();
	for (int i = 0; i < size; i++) cout << "\n\t\t" << vec[i];

	cout << "\n\n  Getting keys containing '.H' in their name: ";
	vec = DB.Keys_With_Name(".H");
	cout << "\n  Retreived keys: ";
	size = vec.size();
	for (int i = 0; i < size; i++) cout << "\n\t\t" << setw(20) << vec[i] << " : " << DB.GetElement(vec[i]).GetName();

	cout << "\n\n  Getting keys containing 'iliti' in their category: ";
	vec = DB.Keys_with_Category("iliti");
	cout << "\n  Retreived keys: ";
	size = vec.size();
	for (int i = 0; i < size; i++) cout << "\n\t\t" << setw(20) << vec[i] << " : " << DB.GetElement(vec[i]).GetCategory();

	cout << "\n\n  Getting keys containing 'ele' in their string data: ";
	vec = DB.Keys_with_String_Data("ele");
	cout << "\n  Retreived keys: ";
	size = vec.size();
	for (int i = 0; i < size; i++) cout << "\n\t\t" << setw(20) << vec[i] << " : " << DB.GetElement(vec[i]).GetData();

	cout << "\n\n  Getting keys created during (2017/25/01 to 2017/02/02): ";
	vec = DB.Keys_Within("2017/25/01", "2017/02/02");
	cout << "\n  Retreived keys: ";
	size = vec.size();
	for (int i = 0; i < size; i++) cout << "\n\t\t" << setw(20) << vec[i] << " : " << DB.GetElement(vec[i]).GetTimeDate();

	cout << "\n\n";
}

void DemoCompoundQueries(string dbPath) {
	Title("Testing compound queries on attached XML", '-');
	NoSQLDB<string> DB;
	DB.ImportXML(dbPath);
	cout << "\n\n  Getting keys with name='Element.h' AND category='NoSQLDB' AND data='ele'";
	vector<string> vec = DB.Query_and_Query(DB.Keys_With_Name("Element.h"),
		DB.Query_and_Query(DB.Keys_with_Category("NoSQLDB"), DB.Keys_with_String_Data("ele")));
	
	int size = vec.size();
	cout << "\n  Retreived keys: ";
	for (int i = 0; i < size; i++) cout << vec[i] << " ";


	cout << "\n\n  Getting keys with name='NoSQLDB.h' OR category='Utilities' OR data='ele'";
	vec = DB.Query_or_Query(DB.Keys_With_Name("NoSQLDB.h"),
		DB.Query_or_Query(DB.Keys_with_Category("Utilities"), DB.Keys_with_String_Data("ele")));

	size = vec.size();
	cout << "\n  Retreived keys: ";
	for (int i = 0; i < size; i++) cout << vec[i] << " ";

	cout << "\n\n";
}

void DemoRegularExpressions(string dbPath) {
	Title("Testing regular expression queries on attached XML", '-');
	NoSQLDB<string> DB;
	DB.ImportXML(dbPath);
	cout << "\n\n  Executing query \"get category='NoSQLDB' and (name='Elememt.h' or timedate='2017/25/01,2017/02/02')\"";
	vector<string> vec = DB.ExecuteQuery("get category='NoSQLDB' and (name='Elememt.h' or timedate='2017/25/01,2017/02/02')");
	int size = vec.size();
	cout << "\n  Retreived keys: ";
	for (int i = 0; i < size; i++) cout << vec[i] << " ";

	cout << "\n\n  Executing query \"add key='dummy' name='dummy' category='dummy' description='dummy' data='dummy'\"";
	DB.ExecuteQuery("add key='dummy' name='dummy' category='dummy' description='dummy' data='dummy'");
	cout << "\n  Displaying added item:\n" << DB.GetElement("dummy").ToString();

	cout << "\n\n  Executing query \"delete name='dummy'\"";
	vec = DB.ExecuteQuery("delete name='dummy'");
	cout << "\n  Deleted key: " << vec[0] << " ";
	cout << endl << endl;
}

int main() {
	try {
		cout << "\n Starting Test Executive to demonstrate all requirements:\n\n";
		Title("Requirement #1: Implemented using C++ using facilities of standard C++ & VS15");

		Title("Demonstrating Requirement #2: implements template class providing key/value paid");
		DemonstrateElement();

		Title("Demonstrating Requirement #3: supports addition/deletion of key/value pairs");
		DemoAditionDeletion();

		Title("Demonstrating Requirement #4: support all editings of a value element");
		DemoEditingFunctions();

		Title("Demonstrating Requirement #5: support persist to XML and import from XML");
		DemoPersistance();

		Title("Demonstrating Requirement #6: support automatic persistance");
		DemoAutoPersistance();

		Title("Demonstrating Requirement #7: support queries");
		DemoQueries("ProjectDB.xml");

		Title("Demonstrating Requirements #8 & #9: support compound queries of (AND/OR) types");
		cout << "\n  Note: making a double query on an earlier query is equivalent to ANDing"
			<< "\n  two queries. In symbols: Query1(Query2Result) = Query1 AND Query2."
			<< "\n  A union between two query results is in fact the OR operation between two"
			<< "\n  queries. In symbols: Query1Result UNION Query2Result = Query1 OR Query2\n\n";
		DemoCompoundQueries("ProjectDB.xml");

		Title("Requirement #10: submitted with XML file that describes the project");
		Title("Requirement #11: accompanied with Test Executive to demonstrate requirements");

		Title("Demonstrating Bonus Requirement #12: support regular expression queries");
		DemoRegularExpressions("ProjectDB.xml");
	}
	catch (...) {
		cout << "\n\n  A problem occured, stopping demonstration. Please check all files";
	}
	return 0;
}

#endif