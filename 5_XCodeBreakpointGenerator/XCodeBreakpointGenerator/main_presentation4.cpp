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

// demonstrates accessing declaration-specific AST-informations. In this case, we add the value of the arguments of the methods we found to the
// breakpoint-message (line 59)

using namespace clang;
using namespace tooling;
using namespace ast_matchers;

static llvm::cl::OptionCategory XCodeBreakpointGeneratorTool("XCodeBreakpointGenerator");

template <typename T> FullSourceRange createSourceRangeForStmt(const T *S, SourceManager &sourceManager) {
  FullSourceRange sourceRange;

  sourceRange.startingLineNumber = sourceManager.getSpellingLineNumber(S->getLocStart());
  sourceRange.endingLineNumber = sourceManager.getSpellingLineNumber(S->getLocEnd());
  sourceRange.startingColumnNumber = sourceManager.getSpellingColumnNumber(S->getLocStart());
  sourceRange.endingColumnNumber = sourceManager.getSpellingColumnNumber(S->getLocEnd());
  sourceRange.filePath = sourceManager.getFilename(sourceManager.getSpellingLoc(S->getLocStart()));

  return sourceRange;
}

std::string getFullMethodName(const ObjCMethodDecl *method) {
  auto className = method->getClassInterface()->getNameAsString();
  auto methodName = method->getSelector().getAsString();
  return std::string((method->isClassMethod() ? "+" : "-")) + "[" + className + " " + methodName + "]";
}

class BreakPointGenerator : public ast_matchers::MatchFinder::MatchCallback {
public:
  virtual void run(const ast_matchers::MatchFinder::MatchResult &Result) {
    const ObjCMethodDecl *method = Result.Nodes.getDeclAs<ObjCMethodDecl>("method");

    auto methodDeclaration = getFullMethodName(method);

    auto logMessage = "entering method " + methodDeclaration;

    // add the values of the parameters to the output of the breakpoint
    for (auto arg : method->parameters()) {
      logMessage += " " + arg->getNameAsString() + ": @" + arg->getNameAsString() + "@";
    }
    logMessage += "\"\n";

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
