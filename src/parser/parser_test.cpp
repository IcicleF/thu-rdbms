#include <cstdlib>

#include "parser.tab.h"
#include "parser_base.h"

int main() {
    return yyparse();
}