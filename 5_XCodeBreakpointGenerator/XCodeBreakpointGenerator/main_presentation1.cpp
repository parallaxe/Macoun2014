//
//  main.cpp
//  ASTMatchersTest
//
//  Created by Hendrik von Prince on 25/08/14.
//  Copyright (c) 2014 Hendrik von Prince. All rights reserved.
//

// demonstrates the basic structure of a clang-tool
// does nothing quite useful, it just checks given sourcefile(s) for correct syntax

#include <iostream>

#include "llvm/Support/CommandLine.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/JSONCompilationDatabase.h"

using namespace clang;
using namespace tooling;

// the provided name will be used in the -help-message of your tool, that is generated by the CommonOptionsParser.
// http://llvm.org/docs/doxygen/html/classllvm_1_1cl_1_1OptionCategory.html
static llvm::cl::OptionCategory XCodeBreakpointGeneratorTool("XCodeBreakpointGenerator");

int main(int argc, const char *argv[]) {
  // "Parses a common subset of command-line arguments, locates and loads a compilation commands database and runs a tool with user-specified action. It also
  // contains a help message for the common command-line options." http://clang.llvm.org/doxygen/classclang_1_1tooling_1_1CommonOptionsParser.html
  CommonOptionsParser OptionsParser(argc, argv, XCodeBreakpointGeneratorTool);

  // "A compilation database allows the user to retrieve all compile command lines that a specified file is compiled with in a project. The retrieved compile
  // command lines can be used to run clang tools over a subset of the files in a project."
  // http://clang.llvm.org/doxygen/classclang_1_1tooling_1_1CompilationDatabase.html
  CompilationDatabase &db = OptionsParser.getCompilations();

  // "Utility to run a FrontendAction over a set of files." http://clang.llvm.org/doxygen/classclang_1_1tooling_1_1ClangTool.html
  ClangTool tool(db, OptionsParser.getSourcePathList());

  // in this case, we just let clang check the syntax
  return tool.run(newFrontendActionFactory<SyntaxOnlyAction>().get());
}
