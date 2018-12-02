#include "parser/parser_base.h"

void yyerror(const char* text) {
    std::cout << text << std::endl;
    exit(0);
}

void issueError(const char* text) {
    std::cout << "Unrecognized char: ";
    yyerror(text);
}