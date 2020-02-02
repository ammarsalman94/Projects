/////////////////////////////////////////////////////////////////////
// TypeTable.cpp - implements TypeTable.h and has test stub        //
// Ver 1.0                                                         //
// Application: Project #2 - Dependency Analysis - CSE-687         //
// Platform:    Dell Inspiron 5520, Win 10, Visual Studio 2015     //
// Author:      Ammar Salman, EECS, Syracuse University            //
//              (313) 788-4694  hoplite.90@hotmail.com             //
/////////////////////////////////////////////////////////////////////


#include <vector>
#include <unordered_map>
#include <string>
#include <exception>
#include <iostream>
#include <iomanip>

#include "TypeTable.h"

#include "../../SharedProjects/Tokenizer/Tokenizer.h"
#include "../../SharedProjects/FileSystem/FileSystem.h"

using namespace std;
using namespace CodeAnalysis;

namespace DependencyAnalysis {
	TypeTable::TypeTable() {
		
	}

	// -----< Copy Constructor >--------------------------------------------------------
	TypeTable::TypeTable(const TypeTable & tt) {
		CodeRepository = tt.CodeRepository;
		GlobalScopeNode = tt.GlobalScopeNode;
		IncludedLibraries = tt.IncludedLibraries;
		DefinedTypes = tt.DefinedTypes;
		UsedTypes = tt.UsedTypes;
	}

	TypeTable::~TypeTable() {
	}

	// -----< Extract Defined Types function >-------------------------------------------
	/* extracts types defined within files */
	void TypeTable::ExtractDefinedTypes(int argc, char** argv) {
		FileList.clear();
		mapKeys.clear();
		qName.clear();
		CodeAnalysisExecutive CodeAnalyzer;
		if (!CodeAnalyzer.ProcessCommandLine(argc, argv)) {
			cout << "\n  Could not parse command line arguments";
			return;
		}
		try {
			CodeAnalyzer.getSourceFiles();
			CodeAnalyzer.processSourceCode(true);
			CodeRepository = Repository::getInstance();
			GlobalScopeNode = CodeRepository->getGlobalScope();
			UpdateFileList(CodeAnalyzer.fileMap());
			DoAction(GlobalScopeNode);
			//TreeWalk(GlobalScopeNode, true);
		}
		catch (exception& ex) {
			cout << "\n  Code Analyzer failed. Details: " << ex.what();
		}
	}

	// -----< Extract Used Types function >-----------------------------------------------
	/* uses tokenizer to extract all types used within a file*/
	void TypeTable::ExtractUsedTypes() {
		for (auto file : FileList) {
			ifstream in(file);
			if (!in.good()) continue;
			try {
				Scanner::Toker toker;
				toker.returnComments(false);
				toker.attach(&in);
				do {
					string tok = toker.getTok();
					if (isValidToken(tok))
						UsedTypes[file].insert(tok);
				} while (in.good());

				/*for (auto type : DefinedTypes[file])
					UsedTypes[file].erase(type.name);*/
			}
			catch (exception &ex) {
				cout << "\n  Tokenizer failed. Details: " << ex.what();
			}
		}

	}


	// -----< Print function >--------------------------------------------------------------
	void TypeTable::Print() {
		cout << "\n\n  Defined Types:";
		for (auto it = DefinedTypes.begin(); it != DefinedTypes.end(); ++it) {
			cout << "\n    " << it->first << ":";
			for (auto val : it->second) {
				cout << "\n        " << setw(10) << left << val.type 
					<< setw(30) << left << val.name << " - Qualified Name - ";
				for (auto name : val.qualifiedName) cout << "::" << name;
			}
			cout << endl;
		}

		cout << "\n\n  Used Types: ";
		for (auto it = UsedTypes.begin(); it != UsedTypes.end(); ++it) {
			cout << "\n    " << it->first << ":";
			size_t i = 0;
			cout << "\n" << setw(10) << "";
			for (auto val : it->second) {
				if (i - 80 < 10 || 80 - i < 10) {
						cout << "\n" << setw(10) << "";
						i = 0;
					}
					cout << " " << val;
					i += val.length();
			}
			cout << "\n";
		}

		cout << "\n\n  Included Libraries: ";
		for (auto it : IncludedLibraries) {
			cout << "\n    " << it.first << ":";
			for (auto val : it.second)
				cout << "\n        " << val;
			cout << "\n";
		}
	}

	// -----< Update FileList function >--------------------------------------------------------
	/* takes FileMap from CodeAnalyzer and builds file list to process later for used types*/
	void TypeTable::UpdateFileList(const CodeAnalysisExecutive::FileMap & fileMap) {
		FileList.clear();
		for (auto vec : fileMap) {
			FileList.reserve(vec.second.size());
			FileList.insert(FileList.begin(), vec.second.begin(), vec.second.end());
		}
	}

	// -----< DoAction function >----------------------------------------------------------------
	/* recursive function that performs DFS through the tree and records everything */
	void TypeTable::DoAction(ASTNode * node) {
		if (node->type() == "namespace" || node->type() == "class" || node->type() == "struct")
			ManageContainer(node);
		else if (node->type() == "enum" || node->type() == "function")
			ManageFunction_Enum(node);
	}

	// -----< Manage Container function >--------------------------------------------------------
	/* takes namespace/class/struct node and searches its declarations then goes to children*/
	void TypeTable::ManageContainer(ASTNode * node) {
		ManageStatements(node->statements());

		if (node->name() == "Global Namespace") {
			for (auto dec : node->decl())
				ManageDeclaration(dec);

			size_t size = node->children().size();
			for (size_t i = 0; i < size; i++)
				DoAction(node->children()[i]);
			return;
		}

		mapKeys.insert(node->path());
		qName.push_back(node->name());
		
		TypeValue tv;
		tv.name = node->name();
		tv.type = node->type();
		tv.pNamespace = node->parentType();
		for (auto name : qName) tv.qualifiedName.push_back(name);

		for (auto key : mapKeys) DefinedTypes[key].push_back(tv);

		for (auto dec : node->decl())
			ManageDeclaration(dec);

		size_t size = node->children().size();
		for (size_t i = 0; i < size; i++)
			DoAction(node->children()[i]);

		qName.pop_back();
		mapKeys.erase(node->path());
	}

	// -----< Manage Function function >--------------------------------------------------------
	void TypeTable::ManageFunction_Enum(ASTNode * node) {
		if (node->name() == "main") return;
		TypeValue tv;
		tv.name = node->name();
		tv.type = node->type();
		tv.pNamespace = node->parentType();
		for (auto name : qName) tv.qualifiedName.push_back(name);
		tv.qualifiedName.push_back(node->name());
		bool partOfClass = true;
		for (auto key : mapKeys) {
			DefinedTypes[key].push_back(tv);
			if (key == node->path()) partOfClass = false;
		}
		if (partOfClass) DefinedTypes[node->path()].push_back(tv);
	}


	// -----< Manage Declaration function >-----------------------------------------------------
	/* takes declaration node and considers only typedefs and using declarations*/
	void TypeTable::ManageDeclaration(DeclarationNode& node) {
		// handle typedefs, it is extremely hard to handle all types of typedefs
		// Therefore I considered the simplest form.
		if (node.declType() == DeclType::typedefDecl){ 
			TypeValue tv;
			tv.name = (*node.pTc())[node.pTc()->length() - 2];
			tv.type = "typedef";
			if (qName.size() > 0) tv.pNamespace = qName[qName.size() - 1];
			for (auto name : qName) tv.qualifiedName.push_back(name);
			tv.qualifiedName.push_back(tv.name);
			for (auto key : mapKeys) DefinedTypes[key].push_back(tv);
		}

		// handle using declarations, they can either be on a whole namespace
		// or part of a namespace, or aliases. Either way, they will be added
		else if (node.declType() == DeclType::usingDecl){ 
			size_t index;
			
			// in case of : using namespace blablabla
			if ((index = node.pTc()->find("namespace")) != node.pTc()->length())
				UsedTypes[node.path()].insert((*node.pTc())[index + 1]);

			// in case of (parts of / nested) namespaces: using std::string
			if ((index = node.pTc()->find("::")) != node.pTc()->length()) {
				while (index < node.pTc()->length()) {
					UsedTypes[node.path()].insert((*node.pTc())[index-1]);
					index = node.pTc()->find("::", index + 2);
				}
				UsedTypes[node.path()].insert((*node.pTc())[node.pTc()->length() - 2]);
			}

			// for aliases, take everything between "using" and "=" as the alias name
			if ((index = node.pTc()->find("=")) != node.pTc()->length()) {
				string temp;
				for (size_t i = 1; i < index; i++)
					temp += (*node.pTc())[i];
				TypeValue tv;
				tv.name = temp;
				tv.type = "alias";
				if (qName.size() > 0) tv.pNamespace = qName[qName.size() - 1];
				for (auto name : qName) tv.qualifiedName.push_back(name);
				tv.qualifiedName.push_back(tv.name);
				for (auto key : mapKeys) DefinedTypes[key].push_back(tv);
			}
		}
	}

	// -----< Manage statements function >--------------------------------------------------------
	/* used to find the included libraries per file */
	void TypeTable::ManageStatements(vector<pair<Scanner::ITokCollection*, string>>& statements) {
		for (auto statement : statements) {
			if ((*statement.first)[1] == "include" && (*statement.first)[2][0] == '\"') {
				string path = (*statement.first)[2];
				path = path.substr(1);
				path = path.substr(0, path.length() - 1);
				FileSystem::Directory::setCurrentDirectory(statement.second.substr(0,statement.second.find_last_of('\\')));
				path = FileSystem::Path::getFullFileSpec(path);
				IncludedLibraries[statement.second].push_back(path);
			}
		}
	}



	// -----< is Valid Token function >---------------------------------------------------
	/* checks whether to accept token or not */
	bool TypeTable::isValidToken(string& tok) {
		// ignore string/char tokens
		if (tok[0] == '\"' || tok[0] == '\'') return false;
		// ignore number tokens
		if (tok.find_first_not_of("0123456789") == string::npos) return false;
		// ignore other stuff defined in vector IgnoredTokens
		for (auto token : IgnoredTokens) {
			if (tok == token) 
				return false;
		}
		return true;
	}

}

#ifdef TEST_TYPETABLE
int main(int argc, char** argv) {
	using namespace DependencyAnalysis;
	try {
		DependencyAnalysis::TypeTable tb;
		tb.ExtractDefinedTypes(argc, argv);
		tb.ExtractUsedTypes();
		tb.Print();
	}
	catch (exception& ex) {
		std::cout << "\n  Type Table failed. Details: " << ex.what();
	}
	return 0;
}
#endif