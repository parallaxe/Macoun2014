# XcodeBreakpointGenerator

XcodeBreakpointGenerator is a clang-tool that i wrote as an example-project for the Macoun-conference in 2014 (https://macoun.de/).
It contains multiple main*.cpp-files, each one representing a step towards the full-functional implementation. The full-fledged
implementation is in main.cpp. If you want to reproduce the steps of the implementation, you have to remove the main.cpp from
the build-phase and add the corresponding main_presentation\<x\>.cpp-file to the build phase. 

It demonstrates the usage of the classes

- ClangTool
- MatchFinder
- MatchCallback
- MatchResult
- DeclarationMatcher
- SourceManager
- RecursiveASTVisitor

As it is mainly an example-project, it is stripped down here and there and does a poor error-handling overall. Feel free to ask questions or open pull requests. 

## Usage
You first have to create a compile_commands.json-file for your project. You can create it by using either xcodebuild and oclint-xcodebuild (described below).

Open a terminal and navigate to your projectfolder that also contains the compile_commands.json-file.

1. Generate the breakpoints for one or multiple files and copy the result into your clipboard
		
		XCodeBreakpointGenerator myProjectFiles/singlefile.m | pbcopy
	or
	
		XCodeBreakpointGenerator myProjectFiles/*.m | pbcopy
	or

		XCodeBreakpointGenerator `find . -name '*.m'` | pbcopy
2. Copy the breakpoints to the corresponding file where Xcode saves its breakpoints. It is located at

		your_project.xcodeproj/xcuserdata/your_username.xcuserdatad/xcdebugger/Breakpoints_v2.xcbkptlist
	If there is no such file, Xcode didn't create it yet. Open your project in Xcode and set a breakpoint, than it should be created.
	Now paste the breakpoints into the Breakpoints_v2.xcbkptlist-file.
	
		<?xml version="1.0" encoding="UTF-8"?>
		<Bucket
		   type = "1"
		   version = "2.0">
		   <Breakpoints>
			   Paste clipboard here
		   </Breakpoints>
		</Bucket>
3. Xcode does not recognize that we changed the file, so it doesn't show up the breakpoints yet. We have to tell him to reload the project

		touch your_project.xcodeproj/*
4. Now the breakpoints should appear in Xcode and take place
5. Start your project and follow your program-flow in the debugger

**Hint** Xcode will crash if you generate a few dozen breakpoints (don't know the exact number).

### Create the compile_commands.json by using oclint-xcodebuild
Get oclint: http://oclint.org/downloads.html

1. Navigate to the folder that contains the xcproject and/or xcworkspace
2. Clean your build-folder
	
		xcodebuild-clean
3. Build your project and write the result into a file named xcodebuild.log. Note that i set an explicit architecture building the project: if your project is configured for multiple architectures, xcodebuild will build them all by default. Thus, all files were referenced multiple times in the xcodebuild.log and the resulting compile_commands.json also would reference each sourcefile multiple times, which we don't want.

		xcodebuild -arch armv7s > xcodebuild.log

	You may have to specify more parameters to build, like a the name of a scheme. xcodebuild -help is your friend.
4. Create the compile_commands.json

		oclint-location/bin/oclint-xcodebuild

That's it, you should now have a compile_commands.json in your directory.

### Create the compile_commands.json by using xctool
Get xctool: 
	
	brew install xctool

1. Navigate to the folder that contains the xcproject and/or xcworkspace
2. Clean your build-folder of the scheme you want to build

		xctool -scheme <scheme name> clean
3. Build your project and create the compile_commands.json
	
		xctool -scheme <scheme name> -reporter json-compilation-database:compilation.json build

## Installation

### Precondition
You have to have clang installed, including its headers and libraries (see http://clang.llvm.org/get_started.html#build). Please not that this project makes use of ASTMatchers that are currently only available in my fork of clang at https://github.com/parallaxe/clang, so you have to use this instead of any official / inofficial clang-repositories.
The settings in clang.xcconfig assume that clang is installed in $HOME/usr/local. If you have installed it another location, you have to replace all occurences of $HOME/usr/local through your installation folder.

### Building + installing
After installing clang, the project should be buildable with Xcode. The Xcode-scheme contains a build-step that will copy the binary to ~/usr/local/bin. Given that your $PATH-variable includes the ~/usr/local/bin path, you can use the tool in your terminal after building it in Xcode.

