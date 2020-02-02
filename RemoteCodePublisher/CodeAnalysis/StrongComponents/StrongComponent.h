#pragma once
/////////////////////////////////////////////////////////////////////
// StrongComponent.h - Strong component analyzer                   //
// Ver 1.0                                                         //
// Application: Project #2 - Dependency Analysis - CSE-687         //
// Platform:    Dell Inspiron 5520, Win 10, Visual Studio 2015     //
// Author:      Ammar Salman, EECS, Syracuse University            //
//              (313) 788-4694  hoplite.90@hotmail.com             //
/////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package is Strong Components analyzer based on NoSQLDB records
* When the TypeAnalysis/DependencyAnalysis finish their work, they 
* will produce a NoSQLDB to record the dependencies. Using this
* package it is very easy to find all strong components.
*
* Public Interface:
* Vertix *v;
* string n = v->name();
* auto conns = v->connections();
*
* StrongComponent sc;
* sc.processNoSqlDb(db);
* sc.extractStrongComponents();
* sc.StrongestComponent_El();
* sc.StrongestComponents_NoSQLDB();
*
* Required Files:
* ---------------
*   - StrongComponent.h, NoSqlDb.h
*
* Build Process:
* --------------
*   devenv DependencyAnalysis.sln /debug rebuild
*
* Maintenance History:
* --------------------
* Ver 1.0 : 4 March 2017
* - first release
*
*/


#include <set>
#include <vector>
#include <string>
#include <unordered_map>
#include <stack>
#include <exception>

#include "../NoSQLDB/NoSqlDb/Element.h"
#include "../NoSQLDB/NoSqlDb/NoSqlDb.h"


namespace Utilities {
	using namespace std;
	using namespace Databases;

	/* 
	 * Vertix class, provides information needed for processing
	 * this class takes information from the NoSQLDB and stores
	 * it for later usage. Each Vertix represents one NoSQLDB
	 * Element.
	 */
	class Vertix {
		string name_;
		// index is set to -1 to have a value representing "undefined"
		// .. according to the Tarjan's algorithm it needs to know it
		int index = -1;
		int lowlink;
		bool onStack;
		// connections are basically the edges for each vertix
		set<string> connections_;

		// these functions take vector<string> which is the 
		// normal output of my NoSQLDB keys list
		void addConnections(set<string>& conns);
		void removeConnections(set<string>& conns);

		// give only StrongComponent full access
		friend class StrongComponent;
	public:
		// expose data as constants to keep them encapsulated well
		const set<string>& connections() const { return connections_; }
		const string& name() const { return name_; }
	};

	// -----< Add Connections function >---------------------------------------
	/* takes vector<string> of keys and adds them to the set of edges*/
	inline void Vertix::addConnections(set<string>& conns) {
		// set automatically gaurantees no duplicates
		connections_.insert(conns.begin(), conns.end());
	}

	// -----< Remove Connections function >------------------------------------
	/* takes vector<string> keys (representing edges) and deletes what found*/
	inline void Vertix::removeConnections(set<string>& conns) {
		for (auto it = connections_.begin(); it != connections_.end(); ++it) {
			for (auto val : conns)
				if ((*it) == val) {
					connections_.erase(it);
					--it;
					// set holds distinct values, no need to continue once found
					break;
				}
		}
	}


	/*
	 * implementation of Tarjan's algorithm on NoSQLDB objects
	 * the design of the database allows for easy representation
	 * of graphs using it. Each key is a Vertix. Each child is
	 * an Edge. This way we have all our needed < V, E >
	 */
	class StrongComponent {
	public:
		// constructor
		StrongComponent() {}
		// DO NOT ALLOW COPY CONSTRUCTION
		StrongComponent(const StrongComponent& whatever) = delete;
		~StrongComponent();
		template<typename T> void processNoSqlDb(NoSQLDB<T>& db);
		void extractStrongComponents();

		Element<int> StrongestComponent_El() { return StrongComponents.GetElement(scKey);}
		NoSQLDB<int> StrongestComponents_NOSQLDB() { return StrongComponents; }

	private:
		void strongConnect(Vertix* v);
		unordered_map<string, Vertix*> graph;
		NoSQLDB<int> StrongComponents;
		stack<Vertix*> Stack;
		size_t index;
		size_t scIndex;
		string scKey;
		size_t scCount;
	};

	// -----< destructor >---------------------------------------------------
	inline StrongComponent::~StrongComponent() {
		for (auto it = graph.begin(); it != graph.end(); ++it)
			delete it->second;
		/* no need to delete anything else as the strongComponents and stack
		 * will only hold pointers that were defined and put inside the graph*/
	}

	// -----< process NoSQLDB function >-------------------------------------
	/* takes NoSQLDB from any type since we only care about keys/children */
	template<typename T>
	inline void StrongComponent::processNoSqlDb(NoSQLDB<T>& db) {
		vector<string> keys = db.GetAllKeys();
		for (auto key : keys) {
			Vertix * vertix = new Vertix();
			vertix->name_ = key;
			vertix->addConnections(db.GetChildren(key));
			graph[key] = vertix;
		}
	}

	// -----< Extract Strong Components function >----------------------------
	/* iterates through the set and calls strong connect recursive function*/
	inline void StrongComponent::extractStrongComponents() {
		scCount = 0;
		index = 0;
		for (auto it = graph.begin(); it != graph.end(); ++it) {
			// -1 means undefined
			if (it->second->index == -1) strongConnect(it->second);
		}

		// analysis is done by now
		// this is to find the largest strong component so we could return it
		int max = 0;
		/*size_t size = strongComponents.size();
		for (size_t i = 0; i < size; ++i)
			if (max < strongComponents[i].size()) {
				max = strongComponents[i].size();
				scIndex = i;
			}*/
		max = 0;
		for (auto key : StrongComponents.GetAllKeys()) {
			if (max < StrongComponents.GetElement(key).GetData()) {
				max = StrongComponents.GetElement(key).GetData();
				scKey = key;
			}
		}
	}


	// -----< Strong Connect function >-------------------------------------
	/* accepts vertix pointer and works recursivly. It behaves exactly
	 * as described in Tarjan's Wikipedia page */
	inline void StrongComponent::strongConnect(Vertix * v) {
		v->index = static_cast<int>(index);
		v->lowlink = static_cast<int>(index);
		++index;
		Stack.push(v);
		v->onStack = true;
		Vertix * w;
		// go through v's edges
		for (auto edge : v->connections()) {
			if (graph.find(edge) == graph.end())
				continue;
			// or //throw InvalidEdgeFound();
			w = graph[edge];
			if (w->index == -1) {
				strongConnect(w);
				v->lowlink = min(v->lowlink, w->lowlink);
			}
			else if (w->onStack)
				v->lowlink = min(v->lowlink, w->index);
		}
		// check to store strong component
		if (v->lowlink == v->index) {
			//vector<Vertix*> strongComponentEntry;
			Element<int> strongComponentElement;
			strongComponentElement.SetKey("component" + to_string(++scCount));
			strongComponentElement.SetName("Strong Component #" + to_string(scCount));
			strongComponentElement.SetCategory("Strong Component");
			strongComponentElement.SetDescription("Children are the component vertices. Data is the size");
			int scSize = 0;
			do {
				scSize++;
				w = Stack.top();
				Stack.pop();
				w->onStack = false;

				strongComponentElement.AddChild(w->name_);
			} while (w != v);
			strongComponentElement.SetData(scSize);

			// add strong components to their containers 
			StrongComponents.AddEntry(strongComponentElement.GetKey(), strongComponentElement);
		}
	}


	// this exception is optional. I can through an exception if a 
	// child relation key from the database did not exist in the db
	// i elected not to use it and ignore the special case
	// but i will keep it for now
	class InvalidEdgeFound : public exception {
		virtual const char* what() throw() {
			return "One of the vertices has an edge to non-existant verix";
		}
	};
}