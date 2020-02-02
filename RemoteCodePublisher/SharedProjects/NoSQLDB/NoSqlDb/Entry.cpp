/////////////////////////////////////////////////////////////////////
// Entry.cpp -  Test stub for NoSqlDb package											 //
// Ver 1.0																												 //
// Application: Project #1 - No-SQL Database - CSE-687 						 //
// Platform:    Dell Inspiron 5520, Win 10, Visual Studio 2015	   //
// Author:      Ammar Salman, EECS, Syracuse University						 //
//              (313) 788-4694  hoplite.90@hotmail.com						 //
/////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <vector>

#include "..\RegularExpression\RegularExpression.h"

#include "Element.h"
#include "NoSqlDb.h"

using namespace std;

// test stub for Element<T> and NoSQLDB<KeyType, Type>
#ifdef TEST_NOSQLDB

Element<string> TestElement() {
	Element<string> e1;
	e1.SetKey("key1");
	e1.SetName("element1");
	e1.SetCategory("C++");
	e1.SetDescription("example element");
	e1.SetData("i love c++");

	e1.AddChild("key2");
	e1.AddChild("key3");

	cout << "\n  Testing Element<string>  ";
	cout << "\n ========================= ";
	cout << "\n  Printing element:";
	cout << "\n" << e1.ToString();
	cout << "\n\n  Printing element to XML:";
	cout << "\n" << e1.ToXMLString();
	
	
	return e1;
}

int main() {
	// testing element
	Element<string> e1 = TestElement();
	Element<string> e2;
	e2.SetKey("key2");
	e2.SetName("element2");
	e2.SetCategory("C#");
	e2.SetDescription("example element two");
	e2.SetData("i love c#");
	e2.AddChild("key3");
	e2.AddChild("key4");

	cout << "\n\n\n  Testing NoSQLDB<string>  ";
	cout << "\n ================================= ";
	cout << "\n\n  Creating database";
	NoSQLDB<string> db;
	cout << "\n\n  Adding elements to database:";
	if (db.AddEntry(e1.GetKey(), e1) && db.AddEntry(e2.GetKey(), e2))
		cout << "\n  Entries added:\n" << e1.ToString() 
				 << "\n" << e2.ToString();

	cout << "\n\n  Importing Database from 'database.xml'";
	db.ImportXML("database.xml");
	cout << "\n" << db.ToString();

	cout << "\n\n  Executing query: { get data='dummy' and (name='ammar' or category='python') }";
	vector<string> output = db.ExecuteQuery("get data='dummy' and (name='ammar' or category='python')");
	cout << "\n  obtained keys and their information:";
	int size = output.size();
	for (int i = 0; i < size; i++)
		cout << "\n  Key: " << output[i] << "; Data: " << db.GetElement(output[i]).GetData()
		<< "; Name: " << db.GetElement(output[i]).GetName() << "; Category: " << db.GetElement(output[i]).GetCategory();

	cout << "\n\n  Adding element using regular expression: ";
	cout << "\n    { add key='key10' name='ammar' category='mongodb' description='testquery'";
	cout << "\n      data = 'something' children = 'key1,key2,key3' }";

	db.ExecuteQuery("add key='key10' name='ammar' category='mongodb' description='testquery' data='something' children='key1,key2,key3'");
	
	cout << "\n  Printing out the new element:\n" << db.GetElement("key10").ToString() << "\n";
	
	db.StoreXML("database2.xml");
	return 0;
	

}

#endif