// stdafx.h - предкомпилированный заголовок MZV-2025
#pragma once

#define CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <cstdarg>
#include <stack>

#include <tchar.h>
#include <windows.h>

#define CHAR_NULL       '\0'
#define CHAR_NEWLINE    '\n'
#define CHAR_TAB        '\t'
#define CHAR_SPACE      ' '
#define CHAR_RETURN     '\r'
#define CHAR_QUOTE      '\''
#define CHAR_DQUOTE     '\"'

#include "Error.h"
#include "Parm.h"
#include "In.h"
#include "IT.h"
#include "LT.h"
#include "FST.h"
#include "Log.h"
#include "StaticLib/StdLib.h"     
#include "Lexer.h"
#include "GRB.h"
#include "MFST.h"
#include "SemanticAnalyzer.h"
#include "PolishNotation.h"
#include "CodeGenerator.h"
#include "Optimizer.h"

using namespace std;