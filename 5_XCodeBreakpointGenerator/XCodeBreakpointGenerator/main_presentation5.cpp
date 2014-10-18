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
#include "clang/Tooling/CompilationDatabase.h"
#include "clang/Tooling/Tooling.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/ASTContext.h"
#include "clang/Basic/SourceManager.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchersInternal.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include "Breakpoint.h"

// demonstrates traversing parts of the AST by using the RecursiveASTVisitor
// adds a log-message to every return-statement of the methods and a breakpoint to the end of each method
// be aware that it contains a bug: the breakpoint at the end of each method is always executed, so
// it will print "returning from method <x>" two times from each method that includes a breakpoint
// this bug is eliminated in main.cpp, i just didn't want to blow this example up by fixing the bug

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

class BreakPointGenerator : public ast_matchers::MatchFinder::MatchCallback {
  std::set<BreakPoint> breakpoints;

  class BreakPointGeneratorForReturnStatements : public RecursiveASTVisitor<BreakPointGeneratorForReturnStatements> {
    std::set<BreakPoint> &breakpoints;
    std::function<FullSourceRange(const Stmt *S)> getSourceRange;
    std::function<std::string(unsigned long startingLineNumber, const std::string &methodName, const ReturnStmt *returnStmt)> generateReturnMessage;
    std::string methodName;
    std::string filePath;

  public:
    BreakPointGeneratorForReturnStatements(decltype(breakpoints) &breakpoints, decltype(getSourceRange) const &getSourceRange,
                                           decltype(generateReturnMessage) const &generateReturnMessage, decltype(methodName) const &methodName,
                                           decltype(filePath) const &filePath)
        : breakpoints(breakpoints), getSourceRange(getSourceRange), generateReturnMessage(generateReturnMessage), methodName(methodName), filePath(filePath) {}

    bool VisitReturnStmt(const clang::ReturnStmt *returnStmt) {
      BreakPoint breakpoint;

      breakpoint.setSourceRange(this->getSourceRange(returnStmt));
      breakpoint.setLandmarkName(this->methodName);

      auto logMessage = this->generateReturnMessage(breakpoint.getSourceRange().startingLineNumber, this->methodName, returnStmt);
      breakpoint.addDebugLogActionWithMessage(logMessage);

      if (this->breakpoints.insert(breakpoint).second) {
        std::cout << breakpoint.asXML() << std::endl;
      }

      return true;
    }
  };

public:
  std::string generateReturnMessage(unsigned long startingLineNumber, const std::string &methodName, const ReturnStmt *mayBeNull) {
    std::stringstream logMessage;
    logMessage << "returning from method " << methodName << " (line " << startingLineNumber << ")";

    if (mayBeNull) {
      auto returnValue = mayBeNull->getRetValue();
      if (returnValue) {
        if (auto declRef = dyn_cast_or_null<DeclRefExpr>(returnValue->IgnoreImpCasts())) {
          if (declRef) {
            logMessage << ", value: @" << declRef->getDecl()->getNameAsString() << "@";
          }
        }
      }
    }
    logMessage << "\"\n";
    return logMessage.str();
  }

  virtual void run(const ast_matchers::MatchFinder::MatchResult &Result) {
    auto method = Result.Nodes.getDeclAs<ObjCMethodDecl>("method");
    auto body = method->getBody();
    if (body) {
      auto methodDeclaration = std::string((method->isClassMethod() ? "+" : "-")) + "[" + method->getClassInterface()->getNameAsString() + " " +
                               method->getSelector().getAsString() + "]";
      auto filePath = Result.Context->getSourceManager().getFilename(Result.Context->getSourceManager().getSpellingLoc(body->getLocStart()));
      std::cerr << "visit " << methodDeclaration << std::endl;
      BreakPoint breakpoint;

      breakpoint.setSourceRange(createSourceRangeForStmt(body, Result.Context->getSourceManager()));
      breakpoint.setLandmarkName(methodDeclaration);

      auto logMessage = "entering method " + methodDeclaration;
      for (auto arg : method->parameters()) {
        logMessage += " " + arg->getNameAsString() + ": @" + arg->getNameAsString() + "@";
      }
      logMessage += "\"\n";

      breakpoint.addDebugLogActionWithMessage(logMessage);

      if (this->breakpoints.insert(breakpoint).second) {
        std::cout << breakpoint.asXML() << std::endl;
      }

      // create breakpoints for every return-statement
      auto sourceRangeGenerator = [&Result](const Stmt *S) { return createSourceRangeForStmt(S, Result.Context->getSourceManager()); };
      auto messageGenerator = [this](unsigned long startingLineNumber, const std::string &methodName, const ReturnStmt *mayBeNull) {
        return this->generateReturnMessage(startingLineNumber, methodName, mayBeNull);
      };
      BreakPointGeneratorForReturnStatements stmts(this->breakpoints, sourceRangeGenerator, messageGenerator, methodDeclaration, filePath);
      // this will generate the breakpoints for the return-statements and print them to stdout
      stmts.TraverseStmt(body);

      // create a breakpoint at the end of the method to catch the cases where a method is determined by a return-statement
      // this triggers the bug as described in line 27
      BreakPoint breakpointAtMethodEnd;

      auto sourceRange = createSourceRangeForStmt(body, Result.Context->getSourceManager());
      // the breakpoint has to appear at the end of the body
      sourceRange.startingLineNumber = sourceRange.endingLineNumber;
      sourceRange.endingColumnNumber = 1;
      sourceRange.startingColumnNumber = 1;
      breakpointAtMethodEnd.setSourceRange(sourceRange);
      breakpointAtMethodEnd.setLandmarkName(methodDeclaration);

      breakpointAtMethodEnd.addDebugLogActionWithMessage(
          this->generateReturnMessage(breakpointAtMethodEnd.getSourceRange().endingLineNumber, methodDeclaration, nullptr));
      if (this->breakpoints.insert(breakpointAtMethodEnd).second) {
        std::cout << breakpointAtMethodEnd.asXML() << std::endl;
      }
    }
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
