#ifndef PARSER_BASE
#define PARSER_BASE

#include <cstdlib>
#include <iostream>

#include "parser/ast.h"

int yylex();
void yyerror(const char*);
void issueError(const char*);

#endif