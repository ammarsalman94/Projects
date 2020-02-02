#ifndef ACTIONSANDRULES_H
#define ACTIONSANDRULES_H
/////////////////////////////////////////////////////////////////////
//  ActionsAndRules.h - declares new parsing rules and actions     //
//  ver 3.5                                                        //
//  Language:      Visual C++ 2015                                 //
//  Platform:      Dell Inspiron 5520, Win10 Pro, VS 2015          //
//  Application:   Project #2 OOD, Spring 2017                     //
//  Author:        Ammar Salman, EECS, Syracuse University         //
//                 (313) 788-4694, hoplite.90@hotmail.com          //
//  Source:        Jim Fawcett, CST 4-187, Syracuse University     //
//                 (315) 443-3948, jfawcett@twcny.rr.com           //
/////////////////////////////////////////////////////////////////////
/*
  Module Operations: 
  ==================
  This module defines several action classes.  Its classes provide 
  specialized services needed for specific applications.  The modules
  Parser, SemiExpression, and Tokenizer, are intended to be reusable
  without change.  This module provides a place to put extensions of
  these facilities and is not expected to be reusable. 

  Public Interface:
  =================
  Toker t(someFile);              // create tokenizer instance
  SemiExp se(&t);                 // create a SemiExp attached to tokenizer
  Parser parser(se);              // now we have a parser
  Rule1 r1;                       // create instance of a derived Rule class
  Action1 a1;                     // create a derived action
  r1.addAction(&a1);              // register action with the rule
  parser.addRule(&r1);            // register rule with parser
  while(se.getSemiExp())          // get semi-expression
    parser.parse();               //   and parse it

  Build Process:
  ==============
  Required files
    - Parser.h, Parser.cpp, ScopeStack.h, ScopeStack.cpp,
      ActionsAndRules.h, ActionsAndRules.cpp, ConfigureParser.cpp,
      ItokCollection.h, SemiExpression.h, SemiExpression.cpp, tokenizer.h, tokenizer.cpp
  Build commands (either one)
    - devenv CodeAnalysis.sln
    - cl /EHsc /DTEST_PARSER parser.cpp ActionsAndRules.cpp \
         semiexpression.cpp tokenizer.cpp /link setargv.obj

  Maintenance History:
  ====================
	ver 3.6 : 02 Mar 2017
	- Fixed bug in function detection when it is part of namespace
	- Reorganized some functions exceeding the metric limits
	- Added support for enum detection
	- Added support for typedef declaration detection
	- Fix for template classes/functions detection 

  ver 3.3 : 26 Feb 2017
  - Fixed bug in public data analysis with changes to rule CppDeclaration
    and its action HandleCppDeclaration.
  - Fixed a minor bug in name collection for operators.  Won't be important
    for dependency analysis.
  ver 3.2 : 28 Aug 16
  - fixed errors in many rules and actions based on lots of testing
  - cleaned up text, removing comments and improving prologues
  ver 3.1 : 23 Aug 16
  - qualified input pointers in rules and actions as const
  - cleaned up code by removing unreachables and commented code, and by simplifying 
  ver 3.0 : 06 Aug 16
  - Added use of AbstrSynTree
  - Added new rules and actions
  - Renamed and modified most of the other actions and rules
  ver 2.1 : 15 Feb 16
  - small functional change to a few of the actions changes display strategy
  - preface the (new) Toker and SemiExp with Scanner namespace
  ver 2.0 : 01 Jun 11
  - added processing on way to building strong code analyzer
  ver 1.1 : 17 Jan 09
  - changed to accept a pointer to interfaced ITokCollection instead
    of a SemiExpression
  ver 1.0 : 12 Jan 06
  - first release

  Planned Changes:
  ================
  C++ lambda detection needs strengthening
*/
//
#include <queue>
#include <string>
#include <sstream>
#include <iomanip>
#include "Parser.h"
#include "../GrammarHelpers/GrammarHelpers.h"
#include "../SemiExp/itokcollection.h"
#include "../ScopeStack/ScopeStack.h"
#include "../Tokenizer/Tokenizer.h"
#include "../SemiExp/SemiExp.h"
#include "../AbstractSyntaxTree/AbstrSynTree.h"
#include "../Logger/Logger.h"
#include "../FileSystem/FileSystem.h"

namespace CodeAnalysis
{  
  ///////////////////////////////////////////////////////////////////
  // Repository instance is used to share resources
  // among all actions.

  enum Language { C /* not implemented */, Cpp, CSharp };

  class Repository  // application specific
  {
  public:
    using Rslt = Logging::StaticLogger<0>;  // use for application results
    using Demo = Logging::StaticLogger<1>;  // use for demonstrations of processing
    using Dbug = Logging::StaticLogger<2>;  // use for debug output
    using Package = std::string;
    using Path = std::string;

  private:
    Language language_ = Language::Cpp;
    Path path_;
    ScopeStack<ASTNode*> stack;
    AbstrSynTree ast;
    ASTNode* pGlobalScope;
    Package package_;
    Scanner::Toker* p_Toker;
    Access currentaccess_ = Access::publ;
    static Repository* instance;
  
  public:
    
    Repository(Scanner::Toker* pToker) : ast(stack)
    {
      p_Toker = pToker;
      pGlobalScope = stack.top();
      instance = this;
    }

    ~Repository()
    {
      Dbug::write("\n  deleting repository");
    }

    Language& language() { return language_; }

    Package& package() { return package_; }

    Path& currentPath() { return path_; }

    Access& currentAccess() { return currentaccess_; }

    static Repository* getInstance() { return instance; }

    ScopeStack<ASTNode*>& scopeStack() { return stack; }

    AbstrSynTree& AST() { return ast; }

    ASTNode* getGlobalScope() { return pGlobalScope; }

    Scanner::Toker* Toker() { return p_Toker; }

    size_t lineCount() 
    { 
      return (size_t)(p_Toker->currentLineCount()); 
    }
  };

  ///////////////////////////////////////////////////////////////
  // rule to detect beginning of scope

  class BeginScope : public IRule
  {
  public:
    bool doTest(const Scanner::ITokCollection* pTc) override
    {
      GrammarHelper::showParseDemo("Test begin scope", *pTc);

      // don't parse SemiExp with single semicolon token

      if (pTc->length() == 1 && (*pTc)[0] == ";")
        return IRule::Stop;

      if (pTc->find("{") < pTc->length())
      {
        doActions(pTc);
      }
      return IRule::Continue;
    }
  };

  ///////////////////////////////////////////////////////////////
  // action to handle scope stack at beginning of scope

  class HandleBeginScope : public IAction
  {
    Repository* p_Repos;
  public:
    HandleBeginScope(Repository* pRepos)
    {
      p_Repos = pRepos;
    }
    void doAction(const Scanner::ITokCollection* pTc) override
    {
      GrammarHelper::showParseDemo("handle begin scope", *pTc);
      //if (p_Repos->scopeStack().size() == 0)
      //  Repository::Demo::write("\n--- empty stack ---");
			//std::cout << "\n\n"<< pTc->show() << "\n\n";

      ASTNode* pElem = new ASTNode;
      pElem->type() = "anonymous";
      pElem->name() = "none";
      pElem->package() = p_Repos->package();
      pElem->startLineCount() = p_Repos->lineCount();
      pElem->endLineCount() = 1;
      pElem->path() = p_Repos->currentPath();
      /*
       * make this ASTNode child of ASTNode on stack top
       * then push onto stack
       */
      p_Repos->AST().add(pElem);
    }
  };

  ///////////////////////////////////////////////////////////////
  // rule to detect end of scope

  class EndScope : public IRule
  {
  public:
    bool doTest(const Scanner::ITokCollection* pTc) override
    {
      GrammarHelper::showParseDemo("Test end scope", *pTc);

      //std::string debug = pTc->show();

      if (pTc->find("}") < pTc->length())
      {
        doActions(pTc);
        return IRule::Stop;
      }
      return IRule::Continue;
    }
  };

  ///////////////////////////////////////////////////////////////
  // action to handle scope stack at end of scope

  class HandleEndScope : public IAction
  {
    Repository* p_Repos;

		void AddEnumTypes(ASTNode * node, std::string& enumType, const Scanner::ITokCollection* pTc) {
			DeclarationNode declNode;
			declNode.access() = p_Repos->currentAccess();
			declNode.package() = p_Repos->package();
			declNode.line() = p_Repos->lineCount();
			declNode.pTc() = pTc->clone();
			declNode.pTc()->clear();
			declNode.pTc()->push_back(enumType);
			declNode.declType() = DeclType::enumVarDecl;
			node->decl().push_back(declNode);
		}

  public:
    using Dbug = Logging::StaticLogger<2>;

    HandleEndScope(Repository* pRepos)
    {
      p_Repos = pRepos;
    }
    void doAction(const Scanner::ITokCollection* pTc) override
    {
      GrammarHelper::showParseDemo("Handle end scope", *pTc);
      //if (p_Repos->scopeStack().size() == 0)
      //{
      //  Repository::Demo::flush();
      //  Repository::Demo::write("\n--- empty stack ---");
      //}
      //ASTNode* pDebug = p_Repos->scopeStack().top();
      //std::string debug1 = pDebug->name();
      //std::string debug2 = pDebug->type();
      //std::string debug3 = pDebug->package();
      //std::string debug = pElem->name();

			


      if (p_Repos->scopeStack().size() == 0)
        return;

      ASTNode* pElem = p_Repos->AST().pop();

			// happens in assignments, and enums ONLY
			// will consider when node is enum 
			if (pTc->length() > 1 && pElem->type() == "enum") {
				size_t index = pTc->find(",");
				while (index < pTc->length()) {
					AddEnumTypes(pElem, (*pTc)[index - 1], pTc);
					index = pTc->find(",", index + 1);
				}
				AddEnumTypes(pElem, (*pTc)[pTc->length() - 2], pTc);
			}

      pElem->endLineCount() = p_Repos->lineCount();
      if (pElem->type() == "class" || pElem->type() == "struct")
        (pElem->endLineCount())++;

      p_Repos->currentAccess() = Access::priv;
    }
  };

  ///////////////////////////////////////////////////////////////
  // rule to detect access for C++

  class DetectAccessSpecifier : public IRule
  {
  public:
    bool doTest(const Scanner::ITokCollection* pTc) override
    {
      GrammarHelper::showParseDemo("Test access spec", *pTc);

      Repository* pRepo = Repository::getInstance();
      if (pRepo->language() != Language::Cpp)
        return IRule::Continue;

      size_t pos = pTc->find(":");
      if (0 < pos && pos < pTc->length())
      {
        const std::string tok = (*pTc)[pos - 1];
        if (tok == "public" || tok == "protected" || tok == "private")
        {
          doActions(pTc);
          return IRule::Stop;
        }
      }
      return IRule::Continue;
    }
  };

  ///////////////////////////////////////////////////////////////
  // action to handle access specifier

  class HandleAccessSpecifier : public IAction
  {
    Repository* p_Repos;

  public:
    HandleAccessSpecifier(Repository* pRepos)
    {
      p_Repos = pRepos;
    }
    void doAction(const Scanner::ITokCollection* pTc) override
    {
      GrammarHelper::showParseDemo("Handle access spec: ", *pTc);

      std::string tok = (*pTc)[pTc->length() - 2];
      Access& access = p_Repos->currentAccess();
      if (tok == "public")
        access = Access::publ;
      else if (tok == "protected")
        access = Access::prot;
      else
        access = Access::priv;
    }
  };

  ///////////////////////////////////////////////////////////////
  // rule to detect preprocessor statements

  class PreprocStatement : public IRule
  {
  public:
    bool doTest(const Scanner::ITokCollection* pTc) override
    {
      GrammarHelper::showParseDemo("Test preproc statement: ", *pTc);

      //std::string debug = pTc->show();

      if (pTc->find("#") < pTc->length())
      {
        doActions(pTc);
        return IRule::Stop;
      }
      return IRule::Continue;
    }
  };

  ///////////////////////////////////////////////////////////////
  // action to add semiexp to scope stack top statements()

  class HandlePreprocStatement : public IAction
  {
    Repository* p_Repos;

  public:
    HandlePreprocStatement(Repository* pRepos)
    {
      p_Repos = pRepos;
    }
    void doAction(const Scanner::ITokCollection* pTc) override
    {
      GrammarHelper::showParseDemo("Handle preproc statement: ", *pTc);

      Scanner::ITokCollection* pClone = pTc->clone();
      ASTNode* pElem = p_Repos->scopeStack().top();
      pElem->statements().push_back(std::make_pair(pClone, p_Repos->currentPath()));

      GrammarHelper::showParse("Preproc Stmt", *pTc);
    }
  };

  ///////////////////////////////////////////////////////////////
  // rule to detect namespace statements

  class NamespaceDefinition : public IRule
  {
  public:
    bool doTest(const Scanner::ITokCollection* pTc) override
    {
      GrammarHelper::showParseDemo("Test namespace definition: ", *pTc);

      const Scanner::ITokCollection& tc = *pTc;
      if (tc[tc.length() - 1] == "{")
      {
        size_t len = tc.find("namespace");
        if (len < tc.length())
        {
          doActions(pTc);
          return IRule::Stop;
        }
      }
      return IRule::Continue;
    }
  };

  ///////////////////////////////////////////////////////////////
  // action to add namespace info to scope stack top

  class HandleNamespaceDefinition : public IAction
  {
    Repository* p_Repos;

  public:
    HandleNamespaceDefinition(Repository* pRepos)
    {
      p_Repos = pRepos;
    }
    void doAction(const Scanner::ITokCollection* pTc) override
    {
      GrammarHelper::showParseDemo("Handle namespace definition: ", *pTc);

      ASTNode* top = p_Repos->scopeStack().top();

      std::string name = (*pTc)[pTc->find("namespace") + 1];
      top->type() = "namespace";
      top->name() = name;
      top->package() = p_Repos->package();

      GrammarHelper::showParse("namespace def", *pTc);
    }
  };

  ///////////////////////////////////////////////////////////////
  // rule to detect class statements

  class ClassDefinition : public IRule
  {
  public:
    bool doTest(const Scanner::ITokCollection* pTc) override
    {
      GrammarHelper::showParseDemo("Test class definition: ", *pTc);

      const Scanner::ITokCollection& tc = *pTc;
      if (tc[tc.length() - 1] == "{")
      {
        size_t len = tc.findLastOf("class");
        if (len < tc.length())
        {
          doActions(pTc);
          return IRule::Stop;
        }
        len = tc.find("interface");
        if (len < tc.length())
        {
          doActions(pTc);
          return IRule::Stop;
        }
      }
      return IRule::Continue;
    }
  };

  ///////////////////////////////////////////////////////////////
  // action to add class info to scope stack top

  class HandleClassDefinition : public IAction
  {
    Repository* p_Repos;

  public:
    HandleClassDefinition(Repository* pRepos)
    {
      p_Repos = pRepos;
    }
    void doAction(const Scanner::ITokCollection* pTc) override
    {
      GrammarHelper::showParseDemo("Handle class definition: ", *pTc);

      p_Repos->currentAccess() = Access::priv;

      ASTNode* top = p_Repos->scopeStack().top();
      size_t typeIndex = pTc->find("class");
      if (typeIndex < pTc->length())
      {
        size_t nameIndex = typeIndex + 1;
        std::string name = (*pTc)[nameIndex];
        top->type() = "class";
        top->name() = name;
        top->package() = p_Repos->package();
        p_Repos->AST().typeMap()[name] = top;
        GrammarHelper::showParse("class def", *pTc);
      }
      else  // C#
      {
        typeIndex = pTc->find("interface");
        size_t nameIndex = typeIndex + 1;
        std::string name = (*pTc)[nameIndex];
        top->type() = "interface";
        top->name() = name;
        top->package() = p_Repos->package();
        p_Repos->AST().typeMap()[name] = top;
        GrammarHelper::showParse("interface def", *pTc);
      }
    }
  };

  ///////////////////////////////////////////////////////////////
  // rule to detect struct statements

  class StructDefinition : public IRule
  {
  public:
    bool doTest(const Scanner::ITokCollection* pTc) override
    {
      GrammarHelper::showParseDemo("Test struct definition: ", *pTc);

      const Scanner::ITokCollection& tc = *pTc;
      if (tc[tc.length() - 1] == "{")
      {
        size_t len = tc.find("struct");
        if (len < tc.length())
        {
          doActions(pTc);
          return IRule::Stop;
        }
      }
      return IRule::Continue;
    }
  };

  ///////////////////////////////////////////////////////////////
  // action to add struct info to scope stack top

  class HandleStructDefinition : public IAction
  {
    Repository* p_Repos;

  public:
    HandleStructDefinition(Repository* pRepos)
    {
      p_Repos = pRepos;
    }
    void doAction(const Scanner::ITokCollection* pTc) override
    {
      GrammarHelper::showParseDemo("Handle struct definition: ", *pTc);

      p_Repos->currentAccess() = Access::publ;

      ASTNode* top = p_Repos->scopeStack().top();

      std::string name = (*pTc)[pTc->find("struct") + 1];
      top->type() = "struct";
      top->name() = name;
      top->package() = p_Repos->package();
      p_Repos->AST().typeMap()[name] = top;

      GrammarHelper::showParse("struct def", *pTc);
    }
  };


	///////////////////////////////////////////////////////////////
	// rule to detect enum statements

	class EnumDefinition : public IRule {
	public:
		bool doTest(const Scanner::ITokCollection* pTc) override {
			GrammarHelper::showParseDemo("Test enum definition: ", *pTc);

			const Scanner::ITokCollection& tc = *pTc;
			if (tc[tc.length() - 1] == "{") {
				size_t len = tc.find("enum");
				if (len < tc.length()) {
					doActions(pTc);
					return IRule::Stop;
				}
			}
			return IRule::Continue;
		}
	};

	///////////////////////////////////////////////////////////////
	// action to add enum info to scope stack top

	class HandleEnumDefinition : public IAction {
		Repository* p_Repos;

	public:
		HandleEnumDefinition(Repository* pRepos) {
			p_Repos = pRepos;
		}
		void doAction(const Scanner::ITokCollection* pTc) override {
			GrammarHelper::showParseDemo("Handle enum definition: ", *pTc);

			p_Repos->currentAccess() = Access::publ;

			ASTNode* top = p_Repos->scopeStack().top();

			std::string name = (*pTc)[pTc->find("enum") + 1];
			top->type() = "enum";
			top->name() = name;
			top->package() = p_Repos->package();
			p_Repos->AST().typeMap()[name] = top;

			GrammarHelper::showParse("enum def", *pTc);
		}
	};

  ///////////////////////////////////////////////////////////////
  // rule to detect C++ function definitions

  class CppFunctionDefinition : public IRule
  {
  public:
    bool doTest(const Scanner::ITokCollection* pTc) override
    {
      Repository* pRepo = Repository::getInstance();
      if (pRepo->language() != Language::Cpp)
        return IRule::Continue;

      GrammarHelper::showParseDemo("Test C++ function definition: ", *pTc);

      const Scanner::ITokCollection& tc = *pTc;
      std::string debug = pTc->show();

      if (tc[tc.length() - 1] == "{")
      {
        if (GrammarHelper::isFunction(*pTc))
        {
          doActions(pTc);
          return IRule::Stop;
        }
      }
      return IRule::Continue;
    }
  };

  ///////////////////////////////////////////////////////////////
  // action to add function info to scope stack top

  class HandleCppFunctionDefinition : public IAction
  {
    Repository* p_Repos;
		void CheckIsDestructor(const Scanner::ITokCollection* pTc, 
			ASTNode *top, size_t &nameIndex, std::string& name) {
			if (nameIndex > 0 && (*pTc)[nameIndex - 1] == "~") {
				--nameIndex;
				name = "~" + name;
			}

			top->type() = "function";
			top->name() = name;
			top->package() = p_Repos->package();

			GrammarHelper::showParse("function def", *pTc);
		}

		void CheckIsOperator(const Scanner::ITokCollection* pTc,
			ASTNode *top, size_t &nameIndex, std::string& name) {
			size_t operIndex = pTc->find("operator");
			if (operIndex < pTc->length()) {
				name = "operator" + (*pTc)[operIndex + 1];// +(*pTc)[operIndex + 2];
				if ((*pTc)[operIndex + 2] != "(")
					name += (*pTc)[operIndex + 2];
				top->name() = name;
				nameIndex = operIndex;
			}
		}

		bool CheckIsMemberOrStructMember(const Scanner::ITokCollection* pTc,
			ASTNode *top, size_t &nameIndex, std::string& name) {
			if (nameIndex > 1 && (*pTc)[nameIndex - 1] == "::") {
				//----< start find class name >--------------------

				std::string className = (*pTc)[nameIndex - 2];

				// is class a template?

				if (className == ">") {
					size_t signIndex = nameIndex - 2;
					size_t startParam = GrammarHelper::findLastBetween(*pTc, "<", 0, signIndex);
					if (0 < startParam && startParam < pTc->length())
						className = (*pTc)[startParam - 1];
				}
				//----< end find class name >----------------------
				/*
				* - this function's ASTNode is at stack top
				* - find ASTNode of function's class
				* - unlink function ASTNode from stack top predecessor
				* - relink function ASTNode to it's class ASTNode
				* - leave function ASTNode on stack top as it may have child nodes
				*/
				ASTNode* pClassNode = p_Repos->AST().find(className);
				if (pClassNode == nullptr)
					return true;
				ASTNode* pFunctNode = p_Repos->scopeStack().top();
				ASTNode* pParentNode = p_Repos->scopeStack().predOfTop();
				pFunctNode->parentType() = pClassNode->type(); // change the type again
				pParentNode->children().pop_back();           // unlink function
				pClassNode->children().push_back(pFunctNode); // relink function
				return true;
			}
			return false;
		}

		void CheckIsLambda(const Scanner::ITokCollection* pTc,
			ASTNode *top, size_t &nameIndex, std::string& name) {
			
			std::string packageName = p_Repos->package();
			std::string ext = FileSystem::Path::getExt(packageName);

			size_t posOpenBracket = pTc->find("[");
			size_t posCloseBracket = pTc->find("]");
			size_t posBrace = pTc->find("{");
			if (posOpenBracket < posCloseBracket && posBrace == (posCloseBracket + 1) && posBrace < pTc->length()) {
				std::string name;
				for (size_t i = posOpenBracket; i < posBrace; ++i)
					name += (*pTc)[i];
				top->name() = name;
				top->type() = "lambda";
			}
		}
  public:
    HandleCppFunctionDefinition(Repository* pRepos)
    {
      p_Repos = pRepos;
    }
    void doAction(const Scanner::ITokCollection* pTc) override
    {
      GrammarHelper::showParseDemo("Handle C++ function definition: ", *pTc);

      //std::string debug = pTc->show();

      ASTNode* top = p_Repos->scopeStack().top();

      size_t nameIndex = pTc->find("(") - 1;
      std::string name = (*pTc)[nameIndex];

      // is function a destructor?
			CheckIsDestructor(pTc, top, nameIndex, name);

      // is function an operator?
			CheckIsOperator(pTc, top, nameIndex, name);
      
      // is function a member of a class or struct?
			if(CheckIsMemberOrStructMember(pTc, top, nameIndex, name)) return;
      
      // is this a lambda?
			CheckIsLambda(pTc, top, nameIndex, name);
    }
  };

  ///////////////////////////////////////////////////////////////
  // rule to detect C# function definitions

  class CSharpFunctionDefinition : public IRule
  {
  public:
    bool doTest(const Scanner::ITokCollection* pTc) override
    {
      //std::string debug = pTc->show();

      Repository* pRepo = Repository::getInstance();
      if (pRepo->language() != Language::CSharp)
        return IRule::Continue;

      GrammarHelper::showParseDemo("Test C# function definition: ", *pTc);

      const Scanner::ITokCollection& tc = *pTc;
      if (tc[tc.length() - 1] == "{")
      {
        Scanner::SemiExp se;
        for (size_t i = 0; i < tc.length(); ++i)
          se.push_back(tc[i]);

        if (GrammarHelper::isFunction(se))
        {
          std::string debug = se.show();
          doActions(&se);
          return IRule::Stop;
        }
      }
      return IRule::Continue;
    }
  };

  ///////////////////////////////////////////////////////////////
  // action to add function info to scope stack top

  class HandleCSharpFunctionDefinition : public IAction
  {
    Repository* p_Repos;

  public:
    HandleCSharpFunctionDefinition(Repository* pRepos)
    {
      p_Repos = pRepos;
    }
    void doAction(const Scanner::ITokCollection* pTc) override
    {
      GrammarHelper::showParseDemo("Handle C# function definition: ", *pTc);

      std::string debug = pTc->show();
      ASTNode* top = p_Repos->scopeStack().top();

      size_t nameIndex = pTc->find("(") - 1;
      std::string name = (*pTc)[nameIndex];

      // is function a destructor?

      if (nameIndex > 0 && (*pTc)[nameIndex - 1] == "~")
      {
        --nameIndex;
        name = "~" + name;
      }

      top->type() = "function";
      top->name() = name;
      top->package() = p_Repos->package();

      GrammarHelper::showParse("function def", *pTc);

      // is function an operator?

      size_t operIndex = pTc->find("operator");
      if (operIndex < pTc->length())
      {
        name = "operator" + (*pTc)[operIndex + 1] + (*pTc)[operIndex + 2];
        top->name() = name;
        nameIndex = operIndex;
      }

      // is lambda?

      size_t posParen = pTc->find("(");
      size_t posBrace = pTc->find("{");
      size_t posEqual = pTc->find("=");
      if (posParen < posBrace && posBrace < pTc->length())
      {
        if (posEqual < pTc->length() - 1 && (*pTc)[posEqual + 1] == ">")
        {
          std::string name;
          for (size_t i = posParen; i <= posBrace; ++i)
            name += (*pTc)[i];
          top->name() = name;
          top->type() = "lambda";
        }
      }
    }
  };

  ///////////////////////////////////////////////////////////////
  // rule to detect control definitions

  class ControlDefinition : public IRule
  {
  public:
    bool doTest(const Scanner::ITokCollection* pTc) override
    {
      GrammarHelper::showParseDemo("Test control definition: ", *pTc);

      const Scanner::ITokCollection& tc = *pTc;
      if (tc[tc.length() - 1] == "{")
      {
        size_t len = tc.find("(");
        if (len < tc.length() && GrammarHelper::isControlKeyWord(tc[len - 1]))
        {
          doActions(pTc);
          return IRule::Stop;
        }
        else if (tc.length() > 1 && GrammarHelper::isControlKeyWord(tc[tc.length() - 2]))
        {
          // shouldn't need this scope since all semiExps have been trimmed
          doActions(pTc);
          return IRule::Stop;
        }
      }
      return IRule::Continue;
    }
  };

  ///////////////////////////////////////////////////////////////
  // action to add control info to scope stack top

  class HandleControlDefinition : public IAction
  {
    Repository* p_Repos;

  public:
    HandleControlDefinition(Repository* pRepos)
    {
      p_Repos = pRepos;
    }
    void doAction(const Scanner::ITokCollection* pTc) override
    {
      GrammarHelper::showParseDemo("Handle control definition: ", *pTc);

      ASTNode* top = p_Repos->scopeStack().top();

      size_t nameIndex = pTc->find("(") - 1;  // if, for, while, switch, catch
      if (nameIndex == pTc->length() - 1)     // do, try - they don't have parens
        nameIndex = pTc->length() - 2;
      std::string name = (*pTc)[nameIndex];
      top->type() = "control";
      top->name() = name;

      GrammarHelper::showParse("control def", *pTc);
    }
  };

  ///////////////////////////////////////////////////////////////
  // action to send semi-expression that starts a function def
  // to console

  class PrintFunction : public IAction
  {
    Repository* p_Repos;
  public:
    using Rslt = Logging::StaticLogger<0>;

    PrintFunction(Repository* pRepos)
    {
      p_Repos = pRepos;
    }
    void doAction(const Scanner::ITokCollection* pTc) override
    {
      std::ostringstream out;
      out << "\n  FuncDef: " << pTc->show().c_str();
      Rslt::write(out.str());
    }
  };

  ///////////////////////////////////////////////////////////////
  // action to send signature of a function def to console

  class PrettyPrintFunction : public IAction
  {
    Repository* p_Repos;
  public:
    using Rslt = Logging::StaticLogger<0>;

    PrettyPrintFunction(Repository* pRepos) : p_Repos(pRepos) {}

    void doAction(const Scanner::ITokCollection* pTc) override
    {
      size_t len = pTc->find(")");

      std::ostringstream out;
      out << "\n\n  Pretty Stmt:    ";
      for (size_t i = 0; i < len + 1; ++i)
        out << (*pTc)[i] << " ";
      out << "\n";
      Rslt::write(out.str());
    }
  };

  ///////////////////////////////////////////////////////////////
  // rule to detect C++ Declaration
  /*
  *  - Declaration ends in semicolon
  *  - has type, name, modifiers & initializers
  *  So:
  *  - strip off modifiers and initializers
  *  - if you have two things left it's a declar, else executable
  */
	class CppDeclaration : public IRule {
		Repository* pRepo;

		void CheckAccess(const Scanner::ITokCollection* pTc) {
			if (pTc->find("private") < pTc->length())
				pRepo->currentAccess() = Access::priv;

			if (pTc->find("protected") < pTc->length())
				pRepo->currentAccess() = Access::prot;

			if (pTc->find("public") < pTc->length() && pRepo->scopeStack().top()->type()!= "function")
				pRepo->currentAccess() = Access::publ;
		}

	public:
		bool doTest(const Scanner::ITokCollection* pTc) override {
			pRepo = Repository::getInstance();
			if (pRepo->language() != Language::Cpp)
				return IRule::Continue;

			GrammarHelper::showParseDemo("Test C++ declaration: ", *pTc);
			std::string debug3 = pTc->show();
			//std::cout << "\n" << debug3;
			Scanner::SemiExp tc;
			tc.clone(*pTc);

			// begin added 2/26/2017
			CheckAccess(pTc);
			// end added 2/26/2017

			if (tc.length() > 0 && tc[0] == "using") {
				doActions(pTc);
				return IRule::Stop;
			}

			std::string debug = tc.show();

			if (tc[tc.length() - 1] == ";" && tc.length() > 2) {
				std::string nextToLast = tc[tc.length() - 2];
				if (nextToLast == "delete" || nextToLast == "default" || nextToLast == "const") {
						// function declaration
						doActions(pTc);
						return IRule::Stop;
				}
				std::string parentType = pRepo->scopeStack().top()->type();

				if (GrammarHelper::isDataDeclaration(tc) || GrammarHelper::isFunctionDeclaration(tc, parentType)) {
					doActions(pTc);
					return IRule::Stop;
				}

				if (parentType != "function") {
					// can't be executable so must be declaration

					doActions(pTc);
					return IRule::Stop;
				}
			}
			return IRule::Continue;
		}
	};

  ///////////////////////////////////////////////////////////////
  // action to add declaration info to scope stack top

	class HandleCppDeclaration : public IAction {
		Repository* p_Repos;

	public:
		HandleCppDeclaration(Repository* pRepos) : p_Repos(pRepos) {}

		void doAction(const Scanner::ITokCollection* pTc) override {
			GrammarHelper::showParseDemo("Handle C++ declaration: ", *pTc);

			// save declaration info in ASTNode

			ASTNode* pCurrNode = p_Repos->scopeStack().top();
			DeclarationNode declNode;
			declNode.access() = p_Repos->currentAccess();
			declNode.pTc() = pTc->clone();
			declNode.package() = p_Repos->package();
			declNode.path() = p_Repos->currentPath();
			declNode.line() = p_Repos->lineCount();

			Scanner::SemiExp se;
			se.clone(*pTc);
			GrammarHelper::removeComments(se);

			if (se[0] == "using") {
				declNode.declType() = DeclType::usingDecl;
				pCurrNode->decl().push_back(declNode);

				GrammarHelper::showParse("using declar", *pTc);
				return;
			}

			if (se[0] == "typedef") {
				declNode.declType() = DeclType::typedefDecl;
				pCurrNode->decl().push_back(declNode);
				GrammarHelper::showParse("typedef declar", *pTc);
				return;
			}

			if (GrammarHelper::isFunctionDeclaration(se, "parentNotFunction")) {
				return;
			}

			if (GrammarHelper::isDataDeclaration(se)) {
				declNode.declType() = DeclType::dataDecl;
				pCurrNode->decl().push_back(declNode);
				GrammarHelper::showParse("data declar", *pTc);
			}
		}
	};

  ///////////////////////////////////////////////////////////////
  // rule to detect C# Declaration
  /*
  *  - Declaration ends in semicolon
  *  - has type, name, modifiers & initializers
  *  So:
  *  - strip of modifiers and initializers
  *  - if you have two things left it's a declar, else executable
  */
	class CSharpDeclaration : public IRule {
	public:
		bool doTest(const Scanner::ITokCollection* pTc) override {
			Repository* pRepo = Repository::getInstance();
			if (pRepo->language() != Language::CSharp)
				return IRule::Continue;

			GrammarHelper::showParseDemo("Test C# declaration: ", *pTc);

			std::string debug = pTc->show();

			Access access = pRepo->currentAccess();
			bool isPublic = false;
			std::string parentType = pRepo->scopeStack().top()->type();

			if (pTc->find("public") < pTc->length() && parentType != "function") {
				isPublic = true;
				pRepo->currentAccess() = Access::publ;
			}

			const Scanner::ITokCollection& tc = *pTc;
			if (tc.length() > 0 && tc[0] == "using") {
				doActions(pTc);
				pRepo->currentAccess() = access;
				return IRule::Stop;
			}

			Scanner::SemiExp se;
			se.clone(*pTc);

			if (GrammarHelper::isDataDeclaration(se)) {
				doActions(pTc);
				pRepo->currentAccess() = access;
				return IRule::Stop;
			}

			if (GrammarHelper::isFunctionDeclaration(se, parentType)) {
				doActions(pTc);
				pRepo->currentAccess() = access;
				return IRule::Stop;
			}
			return IRule::Continue;
		}
	};

  ///////////////////////////////////////////////////////////////
  // action to add declaration info to scope stack top

	class HandleCSharpDeclaration : public IAction {
		Repository* p_Repos;

	public:
		HandleCSharpDeclaration(Repository* pRepos) : p_Repos(pRepos) {}

		void doAction(const Scanner::ITokCollection* pTc) override {
			GrammarHelper::showParseDemo("Handle C# declaration: ", *pTc);

			// store declaration info in ASTNode

			ASTNode* pCurrNode = p_Repos->scopeStack().top();
			DeclarationNode declNode;
			declNode.access() = p_Repos->currentAccess();
			declNode.pTc() = pTc->clone();
			declNode.package() = p_Repos->package();
			declNode.line() = p_Repos->lineCount();

			Scanner::SemiExp se;
			se.clone(*pTc);
			GrammarHelper::removeComments(se);

			if (se[0] == "using") {
				declNode.declType() = DeclType::usingDecl;
				pCurrNode->decl().push_back(declNode);

				GrammarHelper::showParse("using declar", *pTc);
				return;
			}
			//std::string debug = se.show();

			std::string parentType = p_Repos->scopeStack().top()->type();

			if (GrammarHelper::isFunctionDeclaration(se, parentType)) {
				std::string debug = se.show();
				Access adebug = declNode.access();
				declNode.declType() = DeclType::functionDecl;
				pCurrNode->decl().push_back(declNode);
				GrammarHelper::showParse("function declar", *pTc);
			}
			else {
				std::string debug = se.show();
				Access adebug = declNode.access();
				declNode.declType() = DeclType::dataDecl;
				pCurrNode->decl().push_back(declNode);
				GrammarHelper::showParse("data declar", *pTc);
			}
		}
	};

  ///////////////////////////////////////////////////////////////
  // rule to detect C++ Executable

	class CppExecutable : public IRule {
	public:
		bool doTest(const Scanner::ITokCollection* pTc) override {
			Repository* pRepo = Repository::getInstance();
			if (pRepo->language() != Language::Cpp)
				return IRule::Continue;

			GrammarHelper::showParseDemo("Test C++ executable: ", *pTc);

			Scanner::SemiExp tc;
			tc.clone(*pTc);

			if (tc[tc.length() - 1] == ";" && tc.length() > 2) {
				GrammarHelper::removeFunctionArgs(tc);
				GrammarHelper::condenseTemplateTypes(tc);

				// remove modifiers, comments, newlines, returns, and initializers

				Scanner::SemiExp se;
				for (size_t i = 0; i < tc.length(); ++i) {
					if (GrammarHelper::isQualifierKeyWord(tc[i]))
						continue;
					if (se.isComment(tc[i]) || tc[i] == "\n" || tc[i] == "return")
						continue;
					if (tc[i] == "=" || tc[i] == ";") {
						se.push_back(";");
						break;
					}
					else
						se.push_back(tc[i]);
				}
				if (se.length() != 3)  // not a declaration
				{
					doActions(pTc);
					return IRule::Stop;
				}
			}
			return IRule::Continue;
		}
	};

  ///////////////////////////////////////////////////////////////
  // action to display C++ executable info

	class HandleCppExecutable : public IAction {
		Repository* p_Repo;

	public:
		HandleCppExecutable(Repository* pRepo) : p_Repo(pRepo) {}

		void doAction(const Scanner::ITokCollection* pTc) override {
			GrammarHelper::showParseDemo("Handle C++ executable: ", *pTc);

			GrammarHelper::showParse("executable", *pTc);
		}
	};
  ///////////////////////////////////////////////////////////////
  // rule to detect C# Executable

	class CSharpExecutable : public IRule {
	public:
		bool doTest(const Scanner::ITokCollection* pTc) override {
			Repository* pRepo = Repository::getInstance();
			if (pRepo->language() != Language::CSharp)
				return IRule::Continue;

			GrammarHelper::showParseDemo("Test C# executable: ", *pTc);

			const Scanner::ITokCollection& in = *pTc;
			Scanner::SemiExp tc;
			for (size_t i = 0; i < in.length(); ++i)
				tc.push_back(in[i]);

			if (tc[tc.length() - 1] == ";" && tc.length() > 2) {
				GrammarHelper::removeFunctionArgs(tc);
				GrammarHelper::condenseTemplateTypes(tc);

				// remove modifiers, comments, newlines, returns, and initializers

				Scanner::SemiExp se;
				for (size_t i = 0; i < tc.length(); ++i) {
					if (GrammarHelper::isQualifierKeyWord(tc[i]))
						continue;
					if (se.isComment(tc[i]) || tc[i] == "\n" || tc[i] == "return")
						continue;
					if (tc[i] == "=" || tc[i] == ";") {
						se.push_back(";");
						break;
					}
					else
						se.push_back(tc[i]);
				}
				if (se.length() != 3)  // not a declaration
				{
					doActions(pTc);
					return IRule::Stop;
				}
			}
			return IRule::Continue;
		}
	};
  ///////////////////////////////////////////////////////////////
  // action to display C# executable info

	class HandleCSharpExecutable : public IAction {
		Repository* p_Repo;

	public:
		HandleCSharpExecutable(Repository* pRepo) : p_Repo(pRepo) {}

		void doAction(const Scanner::ITokCollection* pTc) override {
			GrammarHelper::showParseDemo("Handle C# executable: ", *pTc);

			Scanner::SemiExp se;
			se.clone(*pTc);
			GrammarHelper::removeComments(se);

			GrammarHelper::showParse("executable", se);
		}
	};
  ///////////////////////////////////////////////////////////////
  // default rule
  // - this is here to catch any SemiExp that didn't parse
  // - We don't have rule for enums, so they are caugth here

	class Default : public IRule {
	public:
		bool doTest(const Scanner::ITokCollection* pTc) override {
			GrammarHelper::showParseDemo("Test default: ", *pTc);

			doActions(pTc);  // catches everything
			return IRule::Stop;
		}
	};
  ///////////////////////////////////////////////////////////////
  // action to display default info

	class HandleDefault : public IAction {
		Repository* p_Repo;

	public:
		HandleDefault(Repository* pRepo) : p_Repo(pRepo) {}

		void doAction(const Scanner::ITokCollection* pTc) override {
			GrammarHelper::showParseDemo("Handle default: ", *pTc);
			GrammarHelper::showParse("default: ", *pTc);
		}
	};
}
#endif
