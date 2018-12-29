#ifndef WHERE_CHECKER_H
#define WHERE_CHECKER_H

#include <cmath>
#include <map>
#include <string>

#include "parser/ast.h"
#include "rm/rm_record.h"

struct ExprType {
    int type;
    union {
        int val;
        float floatval;
        char* strval;
    };
};

bool checkDateStr(std::string strval);

ExprType getColumn(const RMRecord&, std::string, std::string);
bool checkWhere(AstBase*, const std::map<std::string, RMRecord>&);
ExprType calcExpr(AstBase*);

#endif