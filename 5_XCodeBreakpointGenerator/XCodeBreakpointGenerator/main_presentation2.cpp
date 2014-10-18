//
//  main.cpp
//  ASTMatchersTest
//
//  Created by Hendrik von Prince on 25/08/14.
//  Copyright (c) 2014 Hendrik von Prince. All rights reserved.
//

// demonstrates the basic usage of using ASTMatchers (in this case a DeclarationMatcher)
// prints the name of all methods with an implementation of the given sourcefile(s)

#include <iostream>

#include "llvm/Support/CommandLine.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/JSONCompilationDatabase.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchersInternal.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang;
using namespace tooling;
using namespace ast_matchers;

static llvm::cl::OptionCategory XCodeBreakpointGeneratorTool("XCodeBreakpointGenerator");

// "Called when the Match registered for it was successfully found in the AST."
// http://clang.llvm.org/doxygen/classclang_1_1ast__matchers_1_1MatchFinder_1_1MatchCallback.html
class BreakPointGenerator : public MatchFinder::MatchCallback {
public:
  // "Called on every match by the MatchFinder."
  // http://clang.llvm.org/doxygen/classclang_1_1ast__matchers_1_1MatchFinder_1_1MatchCallback.html#a8e2033073b8b59e4dc15bf77cf1cf10e
  void run(const ast_matchers::MatchFinder::MatchResult &Result) {
    // ObjCMethodDecl: "Represents an instance or class method declaration." http://clang.llvm.org/doxygen/classclang_1_1ObjCMethodDecl.html
    // MatchResult: "Contains all information for a given match." http://clang.llvm.org/doxygen/structclang_1_1ast__matchers_1_1MatchFinder_1_1MatchResult.html
    const ObjCMethodDecl *method = Result.Nodes.getDeclAs<ObjCMethodDecl>("method");

    // print the name of the method that has been found
    std::cout << method->getSelector().getAsString() << std::endl;
  }
};

int main(int argc, const char *argv[]) {
  CommonOptionsParser OptionsParser(argc, argv, XCodeBreakpointGeneratorTool);

  CompilationDatabase &db = OptionsParser.getCompilations();

  ClangTool tool(db, OptionsParser.getSourcePathList());
  DeclarationMatcher methodMatcher = objcMethod(isDefinition(), isInMainFile()).bind("method");

  BreakPointGenerator generator;

  // "A class to allow finding matches over the Clang AST." http://clang.llvm.org/doxygen/classclang_1_1ast__matchers_1_1MatchFinder.html
  MatchFinder Finder;
  // "Calls 'Action' with the BoundNodes on every match. Adding more than one 'NodeMatch' allows finding different matches in a single pass over the AST."
  // http://clang.llvm.org/doxygen/classclang_1_1ast__matchers_1_1MatchFinder.html
  Finder.addMatcher(methodMatcher, &generator);

  // create an action for the clang-Tool using the MatchFinder-instance Finder
  return tool.run(newFrontendActionFactory(&Finder).get());
}
