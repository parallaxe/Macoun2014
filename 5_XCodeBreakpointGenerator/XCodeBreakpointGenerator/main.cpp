//
//  main.cpp
//  ASTMatchersTest
//
//  Created by Hendrik von Prince on 25/08/14.
//  Copyright (c) 2014 Hendrik von Prince. All rights reserved.
//

#include <iostream>
#include <regex>

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_os_ostream.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/CompilationDatabase.h"
#include "clang/Rewrite/Frontend/FixItRewriter.h"
#include "clang/Rewrite/Frontend/FrontendActions.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/JSONCompilationDatabase.h"
#include "clang/Tooling/Tooling.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/AST.h"
#include "clang/Basic/SourceManager.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchersInternal.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include "Breakpoint.h"

// adding indentation to the breakpoint-messages
// whenever a method is entered, the indentation is increased by 4, whenever a method left, it is decreased by 4
// the bug from main_presentation5.cpp that logs two return-messages instead of one is solved by defining a variable in lldb named $ignoreBreakpointAtMethodEnd
// the value of the variable (true | false) will determine if the breakpoint at the end of each method will be evaluated
// on every breakpoint that points to a return-statement, the value will be set to true and will be reset to false in the evaluation of the condition of
// the method-ending-breakpoints
// the variables will be initialized in breakpoints that were set in the main-file of the project. Thus, there has to be a file in the compilation_database.json
// that
// matches .*main\\.(m|mm|c|cpp)$ and it has to contain the main-method

const char *const decrementIndentationString = "expr if($ignoreBreakpointAtMethodEnd == false) { (void)[$indentationString setString:[$indentationString "
                                               "substringToIndex:(int)(float)fmax($indentationString.length-4, 0)]]; }";

using namespace clang;
using namespace tooling;
using namespace ast_matchers;

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
    bool containsReturnStmt;

  public:
    BreakPointGeneratorForReturnStatements(decltype(breakpoints) &breakpoints, decltype(getSourceRange) const &getSourceRange,
                                           decltype(generateReturnMessage) const &generateReturnMessage, decltype(methodName) const &methodName,
                                           decltype(filePath) const &filePath)
        : breakpoints(breakpoints), getSourceRange(getSourceRange), generateReturnMessage(generateReturnMessage), methodName(methodName), filePath(filePath),
          containsReturnStmt(false) {}

    bool VisitReturnStmt(const clang::ReturnStmt *returnStmt) {
      this->containsReturnStmt = true;

      BreakPoint breakpoint;

      breakpoint.setSourceRange(this->getSourceRange(returnStmt));
      breakpoint.setLandmarkName(this->methodName);

      auto logMessage = this->generateReturnMessage(breakpoint.getSourceRange().startingLineNumber, this->methodName, returnStmt);
      breakpoint.addDebugLogActionWithMessage(logMessage);
      breakpoint.addDebugCommandActionWithCommand(decrementIndentationString);
      breakpoint.addDebugCommandActionWithCommand("expr (void)($ignoreBreakpointAtMethodEnd = true)");

      if (this->breakpoints.insert(breakpoint).second) {
        std::cout << breakpoint.asXML() << std::endl;
      }

      return true;
    }
  };

public:
  std::string generateReturnMessage(unsigned long startingLineNumber, const std::string &methodName, const ReturnStmt *mayBeNull) {
    std::stringstream logMessage;
    logMessage << "@$indentationString.UTF8String@returning from method " << methodName << " (line " << startingLineNumber << ")";

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

      auto logMessage = "@$indentationString.UTF8String@entering method " + methodDeclaration;
      for (auto arg : method->parameters()) {
        logMessage += " " + arg->getNameAsString() + ": @" + arg->getNameAsString() + "@";
      }
      logMessage += "\"\n";

      breakpoint.addDebugLogActionWithMessage(logMessage);
      breakpoint.addDebugCommandActionWithCommand("expr (void)[$indentationString appendString:@&quot;    &quot;]");

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

      BreakPoint breakpointAtMethodEnd;

      auto sourceRange = createSourceRangeForStmt(body, Result.Context->getSourceManager());
      // the breakpoint has to appear at the end of the body
      sourceRange.startingLineNumber = sourceRange.endingLineNumber;
      sourceRange.endingColumnNumber = 1;
      sourceRange.startingColumnNumber = 1;
      breakpointAtMethodEnd.setSourceRange(sourceRange);
      breakpointAtMethodEnd.setLandmarkName(methodDeclaration);

      breakpointAtMethodEnd.setCondition("$ignoreBreakpointAtMethodEnd ? $ignoreBreakpointAtMethodEnd = false : true");
      breakpointAtMethodEnd.addDebugLogActionWithMessage(
          this->generateReturnMessage(breakpointAtMethodEnd.getSourceRange().endingLineNumber, methodDeclaration, nullptr));
      breakpointAtMethodEnd.addDebugCommandActionWithCommand(decrementIndentationString);
      if (this->breakpoints.insert(breakpointAtMethodEnd).second) {
        std::cout << breakpointAtMethodEnd.asXML() << std::endl;
      }
    }
  }
};

class MainMethodFinder : public ast_matchers::MatchFinder::MatchCallback {
  std::string filePath;

  std::shared_ptr<BreakPoint> indentationVariableInitialiser;

public:
  MainMethodFinder() {}

  void setCurrentFilePath(const std::string &filePath) { this->filePath = filePath; }

  virtual void run(const ast_matchers::MatchFinder::MatchResult &Result) {
    auto mainMethod = Result.Nodes.getDeclAs<FunctionDecl>("mainMethod");

    // set an initial breakpoint to initiate the indentation-variable
    // TODO the indentation-variable should be atomic to be thread-safe
    if (mainMethod) {
      assert(this->indentationVariableInitialiser.get() == nullptr &&
             "multiple main-methods found in one or more files referenced within the compilation-database, can't proceed");

      this->indentationVariableInitialiser = std::unique_ptr<BreakPoint>(new BreakPoint());

      this->indentationVariableInitialiser->setSourceRange(createSourceRangeForStmt(mainMethod, Result.Context->getSourceManager()));
      this->indentationVariableInitialiser->setLandmarkName("main");

      this->indentationVariableInitialiser->addDebugCommandActionWithCommand("expr NSMutableString *$indentationString= [@&quot;&quot; mutableCopy]");
      this->indentationVariableInitialiser->addDebugCommandActionWithCommand("expr BOOL $ignoreBreakpointAtMethodEnd= false");
    }
  }

  const decltype(indentationVariableInitialiser) getIndentationVariableInitialiser() const { return this->indentationVariableInitialiser; }
};

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory XCodeBreakpointGeneratorTool("XCodeBreakpointGenerator");

int main(int argc, const char *argv[]) {
  CommonOptionsParser OptionsParser(argc, argv, XCodeBreakpointGeneratorTool);

  CompilationDatabase &db = OptionsParser.getCompilations();

  // try to find the main-file to find the
  // entry-point to initialise the indentation-variable and the ignoreBreakpointAtMethodEnd-variable
  MainMethodFinder mainMethodFinder;
  for (auto filePath : db.getAllFiles()) {
    if (std::regex_match(filePath, std::regex(".*main\\.(m|mm|c|cpp)$"))) {
      DeclarationMatcher mainMethodMatcher = functionDecl(hasName("main")).bind("mainMethod");

      ClangTool tool(db, filePath);
      MatchFinder finder;
      finder.addMatcher(mainMethodMatcher, &mainMethodFinder);

      mainMethodFinder.setCurrentFilePath(filePath);

      tool.run(newFrontendActionFactory(&finder).get());
    }
  }

  auto indentationVariableInitialiser = mainMethodFinder.getIndentationVariableInitialiser();
  if (indentationVariableInitialiser.get() == nullptr) {
    std::cerr << "could not find the main-method within a file following the scheme main.(m|mm|c|cpp) within your compilation database - can't proceed"
              << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << indentationVariableInitialiser->asXML() << std::endl;

  ClangTool tool(db, OptionsParser.getSourcePathList());
  tool.appendArgumentsAdjuster(new ClangSyntaxOnlyAdjuster());

  DeclarationMatcher methodMatcher = objcMethod(isDefinition(), isInMainFile()).bind("method");

  BreakPointGenerator Printer;
  MatchFinder Finder;
  Finder.addMatcher(methodMatcher, &Printer);

  return tool.run(newFrontendActionFactory(&Finder).get());
}
