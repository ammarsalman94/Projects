#pragma once
/////////////////////////////////////////////////////////////////////
// Element.h -  Database element for NoSqlDb.h                     //
// Ver 1.1                                                         //
// Application: Project #1 - No-SQL Database - CSE-687             //
// Platform:    Dell Inspiron 5520, Win 10, Visual Studio 2015     //
// Author:      Ammar Salman, EECS, Syracuse University            //
//              (313) 788-4694  hoplite.90@hotmail.com             //
/////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package is used as an element for NoSqlDb.h. It has string
* properties, a vector of strings that holds child relationships,
* and template data member which can be "anything".
* 
* It is true the data member can be initialized to anything, but 
* for it to work well with the database the operators << and >> 
* will have to be overloaded for that specific type in order to
* allow for import/export from/to XML files.
*
* Required Files:
* ---------------
*   - Element.h, XmlDocument.h
*
* Build Process:
* --------------
*   devenv NoSQLDB.sln /debug rebuild
*
* Maintenance History:
* --------------------
* ver 1.1 : 03 Mar 2017
* - changed children from vector<string> to set<string>
* ver 1.0 : 28 Jan 2017
* - first release
*
*/

#include <string>
#include <set>
#include <sstream>
#include <ctime>
#include <iostream>
#include <iomanip>
#include "..\XmlDocument\XmlDocument\XmlDocument.h"

namespace Databases {
	using namespace std;

	template<typename T> class Element {
	private:
		string Key;
		string Name;
		string Category;
		string Description;
		string TimeDate;
		set<string> Children;
		T Data;

	public:
		Element();
		~Element();

		void AddChild(string Key);
		void RemoveChild(string Key);

		string ToXMLString() { return ToXMLString(Key); }
		string ToXMLString(string Key);
		string ToString();

		string GetKey() { return Key; }
		void SetKey(string value) { Key = value; }
		string GetName() { return Name; }
		void SetName(string value) { Name = value; }
		string GetCategory() { return Category; }
		void SetCategory(string value) { Category = value; }
		string GetDescription() { return Description; }
		void SetDescription(string value) { Description = value; }
		string GetTimeDate() { return TimeDate; }
		void SetTimeDate(string value) { TimeDate = value; }
		set<string> GetChildren() { return Children; }
		void SetChildren(set<string> value) { Children = value; }
		T GetData() { return Data; }
		void SetData(T value) { Data = value; }

	};

	template <typename T> inline string Element<T>::ToXMLString(string Key) {
		ostringstream oss;
		oss << "\n<Element>";
		oss << "\n\t<Key>" << Key << "</Key>";
		oss << "\n\t<Name>" << Name << "</Name>";
		oss << "\n\t<Category>" << Category << "</Category>";
		oss << "\n\t<Description>" << Description << "</Description>";
		oss << "\n\t<TimeDate>" << TimeDate << "</TimeDate>";
		oss << "\n\t<Children>";
		for (auto child : Children)
			oss << "\n\t\t<Child>" << child << "</Child>";
		oss << "\n\t</Children>";
		oss << "\n\t<Data>" << Data << "\n\t</Data>";
		oss << "\n</Element>";
		return oss.str();
	}

	template<typename T>
	inline string Element<T>::ToString() {
		ostringstream oss;
		oss << setw(20) << "Key : " << Key << endl;
		oss << setw(20) << "Name : " << Name << endl;
		oss << setw(20) << "Category : " << Category << endl;
		oss << setw(20) << "Description : " << Description << endl;
		oss << setw(20) << "Date & Time : " << TimeDate << endl;
		oss << setw(20) << "Children : ";
		for (auto it : Children) 
			oss << "\n" << setw(20) << "" << it;
		oss << endl;
		oss << setw(20) << "Data : " << Data << endl;
		return oss.str();
	}


	template<typename T> inline
		Element<T>::Element() {
		time_t t = time(nullptr);
		tm tm;
		localtime_s(&tm, &t);
		stringstream ss;
		ss << std::put_time(&tm, "%Y/%d/%m  %H:%M:%S");
		TimeDate = ss.str();
	}


	template<typename T> inline
		Element<T>::~Element() {
		Children.clear();
	}

	template<typename T> inline
		void Element<T>::AddChild(string Key) {
		Children.insert(Key);
	}

	template<typename T> inline
		void Element<T>::RemoveChild(string Key) {
		Children.erase(Key);
	}



	// adds support to export vector to string
	template <typename T>
	ostream& operator << (ostream& out, vector<T>& vec) {
		int size = vec.size() - 1;
		for (int i = 0; i < size; i++) {
			out << vec[i] << " ";
		}
		out << vec[size] << "\n";
		return out;
	}

}