#include <cstdlib>

#include "parser.tab.h"
#include "parser_base.h"

AstBase* ast;

int main() {
    return yyparse();
}