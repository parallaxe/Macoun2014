//
//  main.cpp
//  ASTMatchersTest
//
//  Created by Hendrik von Prince on 25/08/14.
//  Copyright (c) 2014 Hendrik von Prince. All rights reserved.
//

#include <iostream>

#include "llvm/Support/CommandLine.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/JSONCompilationDatabase.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchersInternal.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include "Breakpoint.h"

// demonstrates the extraction of sourcelocations of declarations / statements
// writes the generated breakpoints as xml to stdout

using namespace clang;
using namespace tooling;
using namespace ast_matchers;

static llvm::cl::OptionCategory XCodeBreakpointGeneratorTool("XCodeBreakpointGenerator");

// takes a Decl or Stmt and wraps clangs SourceLocation into our own FullSourceRange
// SourceManager: "The SourceManager can be queried for information about SourceLocation objects, turning them into either spelling or expansion locations."
// http://clang.llvm.org/doxygen/classclang_1_1SourceManager.html
template <typename T> FullSourceRange createSourceRangeForStmt(const T *S, SourceManager &sourceManager) {
  FullSourceRange sourceRange;

  // retrieve the spelling-location.
  // "Spelling locations represent where the bytes corresponding to a token came from and expansion locations represent where the location is in the user's
  // view. In the case of a macro expansion, for example, the spelling location indicates where the expanded token came from and the expansion location
  // specifies where it was expanded." http://clang.llvm.org/doxygen/classclang_1_1SourceManager.html
  sourceRange.startingLineNumber = sourceManager.getSpellingLineNumber(S->getLocStart());
  sourceRange.endingLineNumber = sourceManager.getSpellingLineNumber(S->getLocEnd());
  sourceRange.startingColumnNumber = sourceManager.getSpellingColumnNumber(S->getLocStart());
  sourceRange.endingColumnNumber = sourceManager.getSpellingColumnNumber(S->getLocEnd());
  sourceRange.filePath = sourceManager.getFilename(sourceManager.getSpellingLoc(S->getLocStart()));

  return sourceRange;
}

// build a string with the scheme +|-[ClassName MethodName]
std::string getFullMethodName(const ObjCMethodDecl *method) {
  auto className = method->getClassInterface()->getNameAsString();
  auto methodName = method->getSelector().getAsString();
  return std::string((method->isClassMethod() ? "+" : "-")) + "[" + className + " " + methodName + "]";
}

class BreakPointGenerator : public ast_matchers::MatchFinder::MatchCallback {
public:
  void run(const ast_matchers::MatchFinder::MatchResult &Result) {
    const ObjCMethodDecl *method = Result.Nodes.getDeclAs<ObjCMethodDecl>("method");

    auto methodDeclaration = getFullMethodName(method);

    // generate and print the breakpoint for the method
    auto logMessage = "entering method " + methodDeclaration + "\"";
    BreakPoint breakpoint;
    breakpoint.setSourceRange(createSourceRangeForStmt(method->getBody(), Result.Context->getSourceManager()));
    breakpoint.setLandmarkName(methodDeclaration);
    breakpoint.addDebugLogActionWithMessage(logMessage);

    std::cout << breakpoint.asXML() << std::endl;
  }
};

int main(int argc, const char *argv[]) {
  CommonOptionsParser OptionsParser(argc, argv, XCodeBreakpointGeneratorTool);

  CompilationDatabase &db = OptionsParser.getCompilations();

  ClangTool tool(db, OptionsParser.getSourcePathList());
  DeclarationMatcher methodMatcher = objcMethod(isDefinition(), isInMainFile()).bind("method");

  BreakPointGenerator generator;
  MatchFinder Finder;
  Finder.addMatcher(methodMatcher, &generator);

  return tool.run(newFrontendActionFactory(&Finder).get());
}
