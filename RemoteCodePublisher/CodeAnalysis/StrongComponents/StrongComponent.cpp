/////////////////////////////////////////////////////////////////////
// StrongComponent.cpp - Test stub for StrongComponent.h           //
// Ver 1.0                                                         //
// Application: Project #2 - Dependency Analysis - CSE-687         //
// Platform:    Dell Inspiron 5520, Win 10, Visual Studio 2015     //
// Author:      Ammar Salman, EECS, Syracuse University            //
//              (313) 788-4694  hoplite.90@hotmail.com             //
/////////////////////////////////////////////////////////////////////


#ifdef TEST_SCOMPONENT

#include "StrongComponent.h"
#include "../NoSQLDB/NoSqlDb/Element.h"

// The input structure looks like this:
// [pack1] <=> [pack2]
//    ^        |
//    |   <----- 
// [pack3] --> [pack4] <=> [pack5]

int main() {
	using namespace Utilities;
	//generate multiple elements 
	Element<string> el;
	el.SetKey("pack1");
	el.AddChild("pack2");
	el.AddChild("pack3");

	Element<string> el2;
	el2.SetKey("pack2");
	el2.AddChild("pack1");
	el2.AddChild("pack3");

	Element<string> el3;
	el3.SetKey("pack3");
	el3.AddChild("pack1");
	el3.AddChild("pack4");
	
	Element<string> el4;
	el4.SetKey("pack4");
	el4.AddChild("pack5");

	Element<string> el5;
	el5.SetKey("pack5");
	el5.AddChild("pack4");

	// populate the db
	NoSQLDB<string> db;
	db.AddEntry(el.GetKey(), el);
	db.AddEntry(el2.GetKey(), el2);
	db.AddEntry(el3.GetKey(), el3);
	db.AddEntry(el4.GetKey(), el4);
	db.AddEntry(el5.GetKey(), el5);

	// start strong componenet analysis
	StrongComponent sc;
	sc.processNoSqlDb(db);
	sc.extractStrongComponents();

	NoSQLDB<int> sComponents = sc.StrongestComponents_NOSQLDB();

	cout << "\n  All Strong Components:\n";
	cout << sComponents.ToString();
	cout << "\n\n  Strongest Component:\n";
	cout << sc.StrongestComponent_El().ToString();
	cout << endl;

	cout << "\n  Storing strong components to file: StrongComponents.xml";
	sComponents.StoreXML("StrongComponents.xml");
}

#endif
