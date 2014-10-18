//
//  Breakpoint.cpp
//  XCodeBreakpointGenerator
//
//  Created by Hendrik von Prince on 25/08/14.
//  Copyright (c) 2014 Hendrik von Prince. All rights reserved.
//

#include "Breakpoint.h"

BreakPoint::BreakPoint() : shouldBeEnabled(true),
ignoreCount(0),
continueAfterRunningActions(true),
landmarkType(5)
{
  std::tm timeinfo = std::tm();
  timeinfo.tm_year = 101;   // year: 2001
  timeinfo.tm_mon = 0;      // month: january
  timeinfo.tm_mday = 1;     // day: 1st
  std::time_t tt = std::mktime (&timeinfo);
  
  std::chrono::system_clock::time_point referenceDate = std::chrono::system_clock::from_time_t (tt);
  std::chrono::system_clock::duration diff = std::chrono::system_clock::now() - referenceDate;
  this->timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
}

void BreakPoint::updateID() {
  std::stringstream idBuilder;
  idBuilder << this->sourceRange.filePath << this->sourceRange.startingLineNumber;
  
  this->id= idBuilder.str();
}

void BreakPoint::addDebugLogActionWithMessage(const std::string &message) {
  auto action= R"mugi(    <BreakpointActionProxy
      ActionExtensionID = "Xcode.BreakpointAction.Log">
      <ActionContent
        message = ")mugi" + message +
  R"mugi(
        conveyanceType = "0">
      </ActionContent>
    </BreakpointActionProxy>
)mugi";
  this->actions.push_back(action);
}

void BreakPoint::addDebugCommandActionWithCommand(const std::string &command) {
  auto action= R"mugi(<BreakpointActionProxy
      ActionExtensionID = "Xcode.BreakpointAction.DebuggerCommand">
      <ActionContent
        consoleCommand = ")mugi" + command + "\""
  R"mugi(
        conveyanceType = "0">
      </ActionContent>
    </BreakpointActionProxy>
)mugi";
  this->actions.push_back(action);
}

void BreakPoint::setSourceRange(const FullSourceRange &range) {
  this->sourceRange= range;
  
  updateID();
}

decltype(BreakPoint::sourceRange) BreakPoint::getSourceRange() const {
  return this->sourceRange;
}

void BreakPoint::setCondition(const std::string &condition) {
  this->condition = condition;
}

void BreakPoint::setLandmarkName(decltype(landmarkName) landmarkName) {
  this->landmarkName= landmarkName;
}


std::string BreakPoint::asXML() {
  std::string result;
  std::stringstream resultStream;
  
  resultStream << "<BreakpointProxy\n"
  << "  BreakpointExtensionID = \"Xcode.Breakpoint.FileBreakpoint\">\n"
  << "  <BreakpointContent\n"
  << "    shouldBeEnabled = \"" << (this->shouldBeEnabled?"Yes":"No") << "\"\n"
  << "    ignoreCount = \"" << this->ignoreCount << "\"\n";
  if(!condition.empty()) {
    resultStream << "    condition = \"" << this->condition << "\"";
  }
  resultStream << "    continueAfterRunningActions = \"" << (this->continueAfterRunningActions?"Yes":"No") << "\"\n"
  << "    filePath = \"" << this->sourceRange.filePath << "\"\n"
  << "    timestampString = \"" << this->timestamp << "\"\n"
  << "    startingColumnNumber = \"" << this->sourceRange.startingColumnNumber << "\"\n"
  << "    endingColumnNumber = \"" << this->sourceRange.endingColumnNumber << "\"\n"
  << "    startingLineNumber = \"" << this->sourceRange.startingLineNumber << "\"\n"
  << "    endingLineNumber = \"" << this->sourceRange.endingLineNumber << "\"\n"
  << "    landmarkName = \"" << this->landmarkName << "\"\n"
  << "    landmarkType = \"" << this->landmarkType << "\">\n";
  resultStream << "  <Actions>\n";
  for(auto action : this->actions) {
    resultStream << action;
  }
  resultStream << "  </Actions>\n";
  resultStream << "  </BreakpointContent>\n";
  resultStream << "</BreakpointProxy>\n";
  
  return resultStream.str();
}

bool BreakPoint::operator<(const BreakPoint &breakpoint) const {
  return this->id < breakpoint.id;
}
