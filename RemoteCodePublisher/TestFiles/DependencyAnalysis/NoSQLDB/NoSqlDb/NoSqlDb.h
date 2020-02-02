#pragma once
/////////////////////////////////////////////////////////////////////
// NoSqlDb.h - A non-relational database system based on templates //
// Ver 1.1                                                         //
// Application: Project #1 - No-SQL Database - CSE-687             //
// Platform:    Dell Inspiron 5520, Win 10, Visual Studio 2015     //
// Author:      Ammar Salman, EECS, Syracuse University            //
//              (313) 788-4694  hoplite.90@hotmail.com             //
/////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package is a non-relational database system build as course
* project for CSE-687 Object Oriented Design. The package uses 
* unordered_map to hold key-value pairs. Both, key and value, are 
* template types. The value is an Element template. 
*
* This package uses XmlDocument package provided by Dr. Fawcett.
*
* Required Files:
* ---------------
*   - NoSqlDb.h, Element.h, RegularExpression.h, XmlDocument.h
*
* Build Process:
* --------------
*   devenv NoSQLDB.sln /debug rebuild
*
* Maintenance History:
* --------------------
* ver 1.1 : 07 Mar 2017
* - changed GetChildren() to return set<string> instead of vector<string>
* Ver 1.0 : 31 Jan 2017
* - first release
* 
*/

// external dependencies
#include <string>
#include <vector>
#include <iostream>
#include <istream>
#include <fstream>
#include <unordered_map>
#include <algorithm>
#include <set>
#include <iterator>
#include <thread>
#include <Windows.h>
#include <regex>
#include <stdarg.h>

// my own classes
#include "Element.h"
#include "..\RegularExpression\RegularExpression.h"

// professor's classes
#include "..\XmlDocument\XmlDocument\XmlDocument.h"
#include "..\XmlDocument\XmlElement\XmlElement.h"
#include "..\StrHelper.h"

using namespace std;
using namespace XmlProcessing;
using namespace std::regex_constants;

namespace Databases {
	template <typename Type> class NoSQLDB {
	private:
		unordered_map<string, Element<Type>> DB;
		short Max_Allowed_Counts;
		short Counts = 0;
		bool AutoCountSave, AutoTimedSave;
		thread TimerThread;
		string DefaultPath;

	public:
		enum Query { KeyPattern, Name, Category, StringData, TimeDate, Children };
		NoSQLDB();
		NoSQLDB(const NoSQLDB<Type>& db);
		NoSQLDB<Type>& operator=(NoSQLDB<Type>& other);
		~NoSQLDB();



		// -----< add/remove entries to/from database functions >----------------
		bool AddEntry(const string Key, const Element<Type> El);
		bool RemoveEntry(const string Key);
		// -----< add/remove entries to/from database functions >----------------


		// -----< export/import database functions >-----------------------------
		void StoreXML(string Filename);
		void ImportXML(string Filename);
		string ToString();
		// -----< export/import database functions >-----------------------------


		// -----< relationships, metadata and data editing functions >-----------
		bool AddRelation(string Key, string ChildKey);
		bool RemoveRelation(string Key, string ChildKey);
		bool ChangeName(string Key, const string name);
		bool ChangeDescription(string Key, const string des);
		bool ChangeCategory(string Key, const string cat);
		bool ReplaceDataInstance(string Key, Type data);
		// -----< relationships, metadata and data editing functions >-----------


		// -----< query functions >----------------------------------------------
		Element<Type> GetElement(string Key);
		vector<string> GetAllKeys();
		set<string> GetChildren(string Key);
		vector<string> Keys_With_Pattern(string Pattern);
		vector<string> Keys_With_Name(string Name);
		vector<string> Keys_with_Category(string Category);
		vector<string> Keys_with_String_Data(string Data);
		vector<string> Keys_Within(string t1 = "now", string t2 = "now");

		vector<string> ExecuteQuery(string QueryText);
		vector<string> ExecuteQuery(Query qType, string argument, string optional = "");
		vector<string> Query_and_Query(vector<string> result1, vector<string> result2);
		vector<string> Query_or_Query(vector<string> result1, vector<string> result2);
		// -----< query functions >----------------------------------------------

		void SetTimedSave(short Seconds, string Filename);
		void UnsetTimedSave() { AutoTimedSave = false; }

		void SetWriteCounter(short MaxCounts, string Filename);
		void UnsetWriteCounter() { AutoCountSave = false; }

	private:
		void ExecuteSingleQueries(vector<string> &operations,
			vector<string> &arguments, vector<vector<string>> &resultsVectors);
		void ExecuteAddQuery(vector<string>& operations, vector<string>& arguments);
		void Apply_Priorities(vector<int> &priorities, vector<vector<string>> &resultsVectors);
		void TimedAutoSave(short Seconds, string Filename);

	};




	template<typename Type>
	inline NoSQLDB<Type>::NoSQLDB() {
	}

	template<typename Type>
	inline NoSQLDB<Type>::NoSQLDB(const NoSQLDB<Type> & db) {
		for (auto it : db.DB) {
			AddEntry(it.first, it.second);
		}
	}

	template<typename Type>
	inline NoSQLDB<Type>& NoSQLDB<Type>::operator=(NoSQLDB<Type>& other) {
		DB = other.DB;
		return *this;
	}


	template<typename Type>
	inline NoSQLDB<Type>::~NoSQLDB() {
		AutoTimedSave = false;
		if (TimerThread.joinable()) TimerThread.join();
	}


	/// <summary>
	/// Adds database record given key and element
	/// </summary>
	/// <param name="Key">Specifies new key</param>
	/// <param name="El">Specifies the record element</param>
	/// <returns>Boolean (true,false) indicating whether the operation is successful of not</returns>
	template<typename Type>
	inline bool NoSQLDB<Type>::AddEntry(const string Key, const Element<Type> El) {
		if (DB.find(Key) != DB.end())
			return false;
		DB[Key] = El;
		if (++Counts >= Max_Allowed_Counts && AutoCountSave) {
			Counts = 0;
			StoreXML(DefaultPath);
		}
		return true;
	}

	/// <summary>
	/// Removes database record given its key
	/// </summary>
	/// <param name="Key">Specifies the element key to remove</param>
	/// <returns>Boolean (true,false) indicating whether the operation is successful of not</returns>
	template<typename Type>
	inline bool NoSQLDB<Type>::RemoveEntry(const string Key) {
		if (DB.find(Key) == DB.end())
			return false;
		for (auto it = DB.begin(); it != DB.end(); ++it) {
			vector<string> keys = GetChildren(it->first);
			int size = keys.size();
			for (int i = 0; i < size; i++)
				if (keys[i].compare(Key) == 0) {
					keys.erase(keys.begin() + i);
					size = keys.size();
					i--;
				}

		}
		DB.erase(Key);
		if (++Counts >= Max_Allowed_Counts && AutoCountSave) {
			Counts = 0;
			StoreXML(DefaultPath);
		}
		return true;
	}


	/// <summary>
	/// Exports database to XML file
	/// </summary>
	/// <param name="Filename">Specifies filename/path of the XML file to export to</param>
	template<typename Type>
	inline void NoSQLDB<Type>::StoreXML(string Filename) {
		ostringstream oss;
		oss << "<Database>";
		for (auto it = DB.begin(); it != DB.end(); ++it) {
			oss << it->second.ToXMLString(it->first);
		}
		oss << "\n</Database>";
		ofstream File(Filename);
		File << oss.str();
		File.close();
	}

	/// <summary>
	/// Imports database from XML file.
	/// </summary>
	/// <param name="Filename">Specifies the filename/path of the XML</param>
	/// <returns>New database instance containing the imported elements</returns>
	template<typename Type>
	inline void NoSQLDB<Type>::ImportXML(string Filename) {
		try {
			XmlDocument xDoc(Filename, XmlDocument::file);
			vector<XmlDocument::sPtr> elements = xDoc.descendents("Element").select();
			istringstream iss;
			if (elements.size() > 0) {
				string temp; int i = 0;
				for (auto element : elements) {
					Element<Type> *el = new Element<Type>;
					temp = trim(element->children()[0]->children()[0]->value());
					el->SetKey(temp);
					temp = trim(element->children()[1]->children()[0]->value());
					el->SetName(temp);
					temp = trim(element->children()[2]->children()[0]->value());
					el->SetCategory(temp);
					temp = trim(element->children()[3]->children()[0]->value());
					el->SetDescription(temp);
					temp = trim(element->children()[4]->children()[0]->value());
					el->SetTimeDate(temp);

					auto children = element->children()[5]->children();
					int size = children.size();
					for (int i = 0; i < size; i++) {
						temp = trim(children[i]->children()[0]->value());
						el->AddChild(temp);
					}

					temp = trim(element->children()[6]->children()[0]->value());
					iss.str(temp);
					Type type;
					iss >> type;
					el->SetData(type);

					AddEntry(el->GetKey(), *el);
					iss.clear();
				}
			}
		}
		catch (...) { cout << "Unable to access file " << Filename; }
	}

	template<typename Type>
	inline string NoSQLDB<Type>::ToString() {
		ostringstream oss;
		oss << setw(11) << "Database:" << endl;
		for (auto it = DB.begin(); it != DB.end(); ++it) {
			oss << setw(11) << "Record:" << endl << it->second.ToString() << endl;
		}
		return oss.str();
	}



	// ------------------------------------------------------------------------------------------------
	// ---------------< Editing Functions Area >-------------------------------------------------------
	// ------------------------------------------------------------------------------------------------
	/// <summary>
	/// Adds child relation between two keys
	/// </summary>
	/// <param name="Key">Specifies the parent element's key</param>
	/// <param name="ChildKey">Specifies the child element's key</param>
	/// <returns>Boolean (true, false) indicating whether the operation was successful or not</returns>
	template<typename Type>
	inline bool NoSQLDB<Type>::AddRelation(string Key, string ChildKey) {
		if (DB.find(Key) == DB.end()) return false;
		DB[Key].AddChild(ChildKey);
		return true;
	}

	/// <summary>
	/// Removes child relation between two keys
	/// </summary>
	/// <param name="Key">Specifies the parent element's key</param>
	/// <param name="ChildKey">Specifies the child element's key</param>
	/// <returns>Boolean (true, false) indicating whether the operation was successful or not</returns>
	template<typename Type>
	inline bool NoSQLDB<Type>::RemoveRelation(string Key, string ChildKey) {
		if (DB.find(Key) == DB.end()) return false;
		return DB[Key].RemoveChild(ChildKey);
		return true;
	}

	/// <summary>
	/// Changes the name property of an element given its key
	/// </summary>
	/// <param name="Key">Specifies the element key to modify</param>
	/// <param name="name">Specifies the new name property</param>
	/// <returns>Boolean (true, false) indicating whether the operation was successful or not</returns>
	template<typename Type>
	inline bool NoSQLDB<Type>::ChangeName(string Key, const string name) {
		if (DB.find(Key) == DB.end()) return false;
		DB[Key].SetName(name);
		return true;
	}

	/// <summary>
	/// Changes the description property of an element given its key
	/// </summary>
	/// <param name="Key">Specifies the element key to modify</param>
	/// <param name="des">Specifies the new description property</param>
	/// <returns>Boolean (true, false) indicating whether the operation was successful or not</returns>
	template<typename Type>
	inline bool NoSQLDB<Type>::ChangeDescription(string Key, const string des) {
		if (DB.find(Key) == DB.end()) return false;
		DB[Key].SetDescription(des);
		return true;
	}

	/// <summary>
	/// Changes the category property of an element given its key
	/// </summary>
	/// <param name="Key">Specifies the element key to modify</param>
	/// <param name="cat">Specifies the new category property</param>
	/// <returns>Boolean (true, false) indicating whether the operation was successful or not</returns>
	template<typename Type>
	inline bool NoSQLDB<Type>::ChangeCategory(string Key, const string cat) {
		if (DB.find(Key) == DB.end()) return false;
		DB[Key].SetCategory(cat);
		return true;
	}

	/// <summary>
	/// Replaces data instance, with new instance, of an element given its key
	/// </summary>
	/// <param name="Key">Specifies the element key to modify</param>
	/// <param name="data">Specifies the new data instance</param>
	/// <returns>Boolean (true, false) indicating whether the operation was successful or not</returns>
	template<typename Type>
	inline bool NoSQLDB<Type>::ReplaceDataInstance(string Key, Type data) {
		if (DB.find(Key) == DB.end()) return false;
		DB[Key].SetData(data);
		return true;
	}

	// ------------------------------------------------------------------------------------------------
	// ---------------< Editing Functions Area >-------------------------------------------------------
	// ------------------------------------------------------------------------------------------------


	// ------------------------------------------------------------------------------------------------
	// ---------------< Query Functions Area >---------------------------------------------------------
	// ------------------------------------------------------------------------------------------------

	class InvalidQueryException : public exception {
		string details;
	public:
		InvalidQueryException(char* det) { details = det; }
		virtual const char* what() throw() {
			string s = "Cannot process the specified query. Query text is invalid; " + details;
			return s.c_str();
		}
	};

	/// <summary>
	/// Executes a text query composed using regular expressions
	/// </summary>
	/// <param name="QueryText">Specifies the query text (in regular expressions)</param>
	/// <returns>Vector of keys satisfying the query condition(s)</returns>
	/// <example>ExecuteQuery("get name='ammar' or name='odai'")</example>
	template<typename Type>
	inline vector<string> NoSQLDB<Type>::ExecuteQuery(string QueryText) {
		RegularExpression RE(QueryText);
		vector<string> operations = RE.GetOperations();
		vector<string> arguments = RE.GetArguments();
		vector<int> priorities = RE.GetCompoundParameters();
		string cmd = RE.GetCommand();

		// if command is add, just add the element and return empty vector
		if (cmd.compare("add") == 0) {
			ExecuteAddQuery(operations, arguments);
			return vector<string>();
		}

		// for compound queries this will manage each query and combine them to one result
		vector<vector<string>> resultsVectors;

		//process each single query and add its results to the resultsVector
		ExecuteSingleQueries(operations, arguments, resultsVectors);

		// apply priorities in their order: parentheses, AND, OR
		Apply_Priorities(priorities, resultsVectors);

		if (RE.GetCommand().compare("delete") == 0) {
			int size = resultsVectors[0].size();
			for (int i = 0; i < size; i++) RemoveEntry(resultsVectors[0][i]);
		}

		// results vectors will only have 1 vector which holds the final result
		return resultsVectors[0];
	}

	template<typename Type>
	inline vector<string> NoSQLDB<Type>::ExecuteQuery(Query qType, string argument, string optional) {
		if (qType == Children  && DB.find(argument) != DB.end()) return DB[argument].GetChildren();
		vector<string> result;
		string s = "(.*)" + argument + "(.*)|(.*)" + argument + "|" + argument + "(.*)";
		regex rgx(s, ECMAScript | icase);
		for (auto it = DB.begin(); it != DB.end(); ++it) {
			switch (qType) {
			case KeyPattern:
				if (regex_match(it->first, rgx)) result.push_back(it->first);
				break;
			case Name:
				if (regex_match(it->second.GetName(), rgx)) result.push_back(it->first);
				break;
			case Category:
				if (regex_match(it->second.GetCategory(), rgx)) result.push_back(it->first);
				break;
			case StringData:
				try {
					if (regex_match(it->second.GetData(), rgx)) result.push_back(it->first);
				}
				catch (exception) { return vector<string>(); }
				break;
			case TimeDate:
				time_t t1 = time(nullptr), t2 = time(nullptr); tm tm;
				try { t1 = ParseTimeString(argument); }
				catch (exception) { localtime_s(&tm, &t1); }
				try { t2 = ParseTimeString(optional); }
				catch (exception) { localtime_s(&tm, &t2); }
				time_t currentTime = ParseTimeString(it->second.GetTimeDate());
				if (t1 <= currentTime && currentTime <= t2) result.push_back(it->first);
				break;
			}
		}
		return result;
	}

	/// <summary>
	/// Finds an element for a given key
	/// </summary>
	/// <param name="Key">Specifies the key to look for</param>
	template<typename Type>
	inline Element<Type> NoSQLDB<Type>::GetElement(string Key) {
		if (DB.find(Key) != DB.end())
			return DB[Key];
		return Element<Type>();
	}

	template<typename Type>
	inline vector<string> NoSQLDB<Type>::GetAllKeys() {
		vector<string> v;
		for (auto it = DB.begin(); it != DB.end(); ++it)
			v.push_back(it->first);
		return v;
	}

	/// <summary>
	/// Finds all children of an element given its key
	/// </summary>
	/// <param name="Key">Specifies the key used to reach the element</param>
	/// <returns>Vector of children keys for the given element</returns>
	template<typename Type>
	inline set<string> NoSQLDB<Type>::GetChildren(string Key) {
		if (DB.find(Key) == DB.end()) return set<string>();
		return DB[Key].GetChildren();
	}


	/// <summary>
	/// Finds all keys that follow a specified pattern
	/// </summary>
	/// <param name="Pattern">Spcifies the pattern to follow</param>
	/// <returns>Vector of all keys satisfying the condition</returns>
	template<typename Type>
	inline vector<string> NoSQLDB<Type>::Keys_With_Pattern(string Pattern) {
		vector<string> result;
		string s = "(.*)" + Pattern + "(.*)|(.*)" + Pattern + "|" + Pattern + "(.*)";
		regex rgx(s, ECMAScript | icase);

		for (auto it = DB.begin(); it != DB.end(); ++it)
			if (regex_match(it->first, rgx)) result.push_back(it->first);

		// if nothing matched the pattern return all keys
		if (result.size() == 0)
			for (auto it = DB.begin(); it != DB.end(); ++it) result.push_back(it->first);
		return result;
	}


	/// <summary>
	/// Finds all elements in the database which have a specified name
	/// </summary>
	/// <param name="Name">Specifies the name to search for</param>
	/// <returns>Vector of all keys satisfying the condition</returns>
	template<typename Type>
	inline vector<string> NoSQLDB<Type>::Keys_With_Name(string Name) {
		vector<string> result;
		string s = "(.*)" + Name + "(.*)|(.*)" + Name + "|" + Name + "(.*)";
		regex rgx(s, ECMAScript | icase);
		for (auto it = DB.begin(); it != DB.end(); ++it)
			if (regex_match(it->second.GetName(), rgx)) result.push_back(it->first);

		return result;
	}

	/// <summary>
	/// Finds all elements in the database that contain a specified category
	/// </summary>
	/// <param name="Category">Specifies the search pattern for the category property</param>
	/// <returns>Vector of keys that satisfy the condition</returns>
	template<typename Type>
	inline vector<string> NoSQLDB<Type>::Keys_with_Category(string Category) {
		vector<string> result;
		string s = "(.*)" + Category + "(.*)|(.*)" + Category + "|" + Category + "(.*)";
		regex rgx(s, ECMAScript | icase);
		for (auto it = DB.begin(); it != DB.end(); ++it)
			if (regex_match(it->second.GetCategory(), rgx)) result.push_back(it->first);
		return result;
	}


	/// <summary>
	/// Special for databases with string datatypes. 
	/// Finds all keys of which elements contain a specified string in their string data item
	/// </summary>
	/// <param name="Data">Specifies the string pattern to search for in the data items</param>
	/// <returns>Vector of all keys satisfying the condition</returns>
	template<typename Type>
	inline vector<string> NoSQLDB<Type>::Keys_with_String_Data(string Data) {
		try {
			vector<string> result;
			string s = "(.*)" + Data + "(.*)|(.*)" + Data + "|" + Data + "(.*)";
			regex rgx(s, ECMAScript | icase);
			for (auto it = DB.begin(); it != DB.end(); ++it)
				if (regex_match(it->second.GetData(), rgx)) result.push_back(it->first);
			return result;
		}
		// if datatype is not string, finding something will throw an exception, so return
		catch (exception ex) {
			cout << ex.what();
			return vector<string>();
		}
	}

	/// <summary>
	/// Finds all keys for elements created between the given two dates.
	/// </summary>
	/// <param name="t1">Specifies the first date. If set to default "" then current date is set instead</param>
	/// <param name="t2">Specifies the second date. If set to default "" then current date is set instead</param>
	/// <returns>Vector of all keys in the specified time interval</returns>
	template<typename Type>
	inline vector<string> NoSQLDB<Type>::Keys_Within(string t1, string t2) {
		time_t time1 = time(nullptr), time2 = time(nullptr);
		tm tm;
		try {
			time1 = ParseTimeString(t1);
		}
		catch (exception) {
			localtime_s(&tm, &time1);
		}
		try {
			time2 = ParseTimeString(t2);
		}
		catch (exception) {
			localtime_s(&tm, &time2);
		}
		vector<string> result;
		for (auto it = DB.begin(); it != DB.end(); ++it) {
			time_t elementTime = ParseTimeString(it->second.GetTimeDate());
			if (elementTime >= time1 && elementTime <= time2) result.push_back(it->first);
		}
		return result;
	}


	/// <summary>
	/// Performs intersection operation between results of two other queries
	/// </summary>
	/// <param name="result1">First query keys collection</param>
	/// <param name="result2">Second query keys collection</param>
	/// <returns>Vector of keys holding distinct keys based on the intersection of the input vectors</returns>
	template<typename Type>
	inline vector<string> NoSQLDB<Type>::Query_and_Query
	(vector<string> result1, vector<string> result2) {
		set<string> s1(result1.begin(), result1.end());
		set<string> s2(result2.begin(), result2.end());
		vector<string> result;
		set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(), back_inserter(result));
		return result;
	}

	/// <summary>
	/// Performs union operation between results of two other queries
	/// </summary>
	/// <param name="result1">First query keys collection</param>
	/// <param name="result2">Second query keys collection</param>
	/// <returns>Vector of keys holding distinct keys based on the union of the input results</returns>
	template<typename Type>
	inline vector<string> NoSQLDB<Type>::Query_or_Query
	(vector<string> result1, vector<string> result2) {
		set<string> s1(result1.begin(), result1.end());
		set<string> s2(result2.begin(), result2.end());
		vector<string> result;
		set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), back_inserter(result));
		return result;
	}


	// ------------------------------------------------------------------------------------------------
	// ---------------< Query Functions Area >---------------------------------------------------------
	// ------------------------------------------------------------------------------------------------

	template<typename Type>
	inline void NoSQLDB<Type>::SetTimedSave(short Seconds, string Filename) {
		if (TimerThread.joinable()) TimerThread.join();
		AutoTimedSave = true;
		TimerThread = thread(&NoSQLDB<Type>::TimedAutoSave, this, Seconds, Filename);
	}

	template<typename Type>
	inline void NoSQLDB<Type>::SetWriteCounter(short MaxCounts, string Filename) {
		Max_Allowed_Counts = MaxCounts;
		DefaultPath = Filename;
		AutoCountSave = true;
	}

	// ------------------------------------------------------------------------------------------------
	// ---------------< Query Processing Functions Area >----------------------------------------------
	// ------------------------------------------------------------------------------------------------



	/// <summary>
	/// Executes separate queries given their information
	/// </summary>
	/// <param name="operations">Holds the operations to consider</param>
	/// <param name="arguments">Holds the arguments for the operations</param>
	/// <param name="resultsVectors">Vector of vectors of keys to hold each operation result</param>
	template<typename Type>
	inline void NoSQLDB<Type>::ExecuteSingleQueries(vector<string>& operations,
		vector<string>& arguments, vector<vector<string>> &resultsVectors) {
		int size = operations.size();
		for (int i = 0; i < size; i++) {
			if (operations[i].compare("name") == 0)
				resultsVectors.push_back(Keys_With_Name(arguments[i]));
			else if (operations[i].compare("category") == 0)
				resultsVectors.push_back(Keys_with_Category(arguments[i]));
			else if (operations[i].compare("data") == 0)
				resultsVectors.push_back(Keys_with_String_Data(arguments[i]));
			else if (operations[i].compare("timedate") == 0) {
				stringstream ss(arguments[i]);
				string t1 = "", t2 = "";
				getline(ss, t1, ',');
				getline(ss, t2);
				resultsVectors.push_back(Keys_Within(t1, t2));
			}
		}
	}

	// special for execution of add command when issued by regular expression
	template<typename Type>
	inline void NoSQLDB<Type>::ExecuteAddQuery(vector<string>& operations, vector<string>& arguments) {
		try {
			int size = operations.size();
			Element<string> el;
			for (int i = 0; i < size; i++) {
				if (operations[i].compare("key") == 0) el.SetKey(arguments[i]);
				if (operations[i].compare("name") == 0) el.SetName(arguments[i]);
				if (operations[i].compare("category") == 0) el.SetCategory(arguments[i]);
				if (operations[i].compare("description") == 0) el.SetDescription(arguments[i]);
				if (operations[i].compare("children") == 0) {
					stringstream ss(arguments[i]);
					string child;
					while (getline(ss, child, ',')) {
						child = trim(child);
						el.AddChild(child);
					}
				}
				if (operations[i].compare("data") == 0) {
					Type data;
					istringstream iss(arguments[i]);
					iss >> data;
					el.SetData(data);
				}
			}
			if (el.GetKey().empty())
				throw new InvalidQueryException("key was not specified");
			AddEntry(el.GetKey(), el);
		}
		catch (InvalidQueryException ex) {
			cout << ex.what();
		}
	}

	/// <summary>
	/// Applies intersections then unions of query results given by different queries
	/// </summary>
	/// <param name="compound">Holds booleans (true,false) which indicate 
	/// whether to do intersection or union, respectively</param>
	/// <param name="resultsVectors">Vector of vectors of keys to hold each operation result</param>
	template<typename Type>
	inline void NoSQLDB<Type>::Apply_Priorities(vector<int>& priorities,
		vector<vector<string>>& resultsVectors) {
		int size, k;
		// priorities are from 1 (highest) to 4 (lowest) 
		// p=1 and p=3 are AND operations
		// p=2 and p=4 are OR operations
		for (int p = 1; p <= 4; p++) {
			size = priorities.size();
			k = 0;
			for (int i = 0; i < size; i++) {
				vector<string> result;
				if (priorities[i] == p) {
					// perform AND (intersection) between the two given result vectors
					if (p % 2 != 0) result = Query_and_Query(resultsVectors[i - k], resultsVectors[i + 1 - k]);
					else result = Query_or_Query(resultsVectors[i - k], resultsVectors[i + 1 - k]);
					// remove the processed vectors
					resultsVectors.erase(resultsVectors.begin() + i - k);
					resultsVectors.erase(resultsVectors.begin() + i - k);
					// add the result of processing in place of the removed vectors
					resultsVectors.insert(resultsVectors.begin() + i - k, result);
					k++;
				}
			}
			// update priorities and remove the processed priority
			for (int i = size - 1; i > 0; i--)
				if (priorities[i] == p)
					priorities.erase(priorities.begin() + i);
		}
	}

	template<typename Type>
	inline void NoSQLDB<Type>::TimedAutoSave(short Seconds, string Filename) {
		while (true) {
			Sleep(Seconds);
			if (!AutoTimedSave) break;
			StoreXML(Filename);
		}
	}



	// ------------------------------------------------------------------------------------------------
	// ---------------< Query Processing Functions Area >----------------------------------------------
	// ------------------------------------------------------------------------------------------------


	// ------------------------------------------------------------------------------------------------
	// ---------------< Accessory Functions Area >-----------------------------------------------------
	// ------------------------------------------------------------------------------------------------


	// timedate parser, takes string datetime and returns time_t of it
	inline time_t ParseTimeString(string TimeString) {
		int years, month, day, hour = 0, minute = 0, second = 0;

		years = stoi(TimeString.substr(0, 4));
		day = stoi(TimeString.substr(5, 2));
		month = stoi(TimeString.substr(8, 2));
		if (TimeString.length() == 14)
			hour = stoi(TimeString.substr(12, 2));
		if (TimeString.length() == 17)
			minute = stoi(TimeString.substr(15, 2));
		if (TimeString.length() == 20)
			second = stoi(TimeString.substr(18, 2));

		time_t t;
		tm timedate;

		time(&t);
		localtime_s(&timedate, &t);
		timedate.tm_year = years - 1900;
		timedate.tm_mon = month - 1;
		timedate.tm_mday = day;

		timedate.tm_hour = hour;
		timedate.tm_min = minute;
		timedate.tm_sec = second;

		t = mktime(&timedate);

		return t;
	}

	// character comparison
	inline bool icompare_pred(unsigned char a, unsigned char b) {
		return std::tolower(a) == std::tolower(b);
	}

	// compares two strings
	inline bool icompare(std::string const& a, std::string const& b) {
		if (a.length() != b.length()) return false;
		return std::equal(b.begin(), b.end(), a.begin(), icompare_pred);
	}



	// ------------------------------------------------------------------------------------------------
	// ---------------< Accessory Functions Area >-----------------------------------------------------
	// ------------------------------------------------------------------------------------------------

	// adds support to import full string (including whitespaces and newlines)
	inline istream& operator >> (istream& in, string& ss) {
		ostringstream oss;
		string temp;
		while (in.good()) {
			getline(in, temp);
			oss << temp << "\n";
		}
		ss = oss.str().substr(0, oss.str().size() - 1);
		return in;
	}

	// adds support to import vector from string
	template <typename T>
	inline istream& operator >> (istream& in, vector<T>& vec) {
		istringstream temp;
		string s;
		while (in.good()) {
			T val;
			getline(in, s);
			temp.str(s);
			while (temp.good()) {
				temp >> val;
				vec.push_back(val);
			}
			s.clear();
			temp.clear();
		}
		return in;
	}

}