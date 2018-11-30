#ifndef PARSER_TREE
#define PARSER_TREE

#include <vector>

class QueryResponse;
class AstBase {
    protected:
        int type;
        union {
            int val;
            float floatval;
            char* strval;
        };

        AstBase* parent;
        std::vector<AstBase*> childs;
    
    public:
        AstBase(int type = 0) : type(type) { 
            childs.clear();
        }
        virtual void printTree() = 0;
};

extern AstBase* ast;

const int L_INT = 500;
const int L_DECIMAL = L_INT + 1;
const int L_STRING = L_DECIMAL + 1;
const int L_NULL = L_STRING + 1;

const int AST_KEYWORD = 1000;
const int AST_LITERAL = AST_KEYWORD + 1;
const int AST_IDENTIFIER = AST_LITERAL + 1;
const int AST_TOPLEVEL = AST_IDENTIFIER + 1;
const int AST_SHOWDB = AST_TOPLEVEL + 1;
const int AST_SETPARAM = AST_SHOWDB + 1;
const int AST_CREATEDB = AST_SETPARAM + 1;
const int AST_DROPDB = AST_CREATEDB + 1;
const int AST_USEDB = AST_DROPDB + 1;
const int AST_SHOWTABLES = AST_USEDB + 1;
const int AST_CREATETABLE = AST_SHOWTABLES + 1;
const int AST_DROPTABLE = AST_CREATETABLE + 1;
const int AST_DESC = AST_DROPTABLE + 1;
const int AST_INSERT = AST_DESC + 1;
const int AST_DELETE = AST_INSERT + 1;
const int AST_UPDATE = AST_DELETE + 1;
const int AST_SELECT = AST_UPDATE + 1;
const int AST_CREATEINDEX = AST_SELECT + 1;
const int AST_DROPINDEX = AST_CREATEINDEX + 1;
const int AST_FIELDLIST = AST_DROPINDEX + 1;
const int AST_FIELD = AST_FIELDLIST + 1;
const int AST_PRIMKEYDECL = AST_FIELD + 1;
const int AST_FOREKEYDECL = AST_PRIMKEYDECL + 1;
const int AST_TYPE = AST_FOREKEYDECL + 1;
const int AST_VALLISTS = AST_TYPE + 1;
const int AST_VALLIST = AST_VALLISTS + 1;
const int AST_INTVAL = AST_VALLIST + 1;
const int AST_FLOATVAL = AST_INTVAL + 1;
const int AST_STRVAL = AST_FLOATVAL + 1;
const int AST_NULL = AST_STRVAL + 1;
const int AST_WHERECLAUSE = AST_NULL + 1;
const int AST_COL = AST_WHERECLAUSE + 1;
const int AST_EXPR = AST_COL + 1;
const int AST_SETCLAUSE = AST_EXPR + 1;
const int AST_SET = AST_SETCLAUSE + 1;
const int AST_SELECTOR = AST_SET + 1;
const int AST_COLLIST = AST_SELECTOR + 1;
const int AST_IDENTLIST = AST_COLLIST + 1;

const int TYPE_INT = 2000;
const int TYPE_CHAR = TYPE_INT + 1;
const int TYPE_VARCHAR = TYPE_CHAR + 1;
const int TYPE_DATE = TYPE_VARCHAR + 1;
const int TYPE_FLOAT = TYPE_FLOAT + 1;

const int WHERE_AND = 3000;
const int WHERE_OR = WHERE_AND + 1;
const int WHERE_EQ = WHERE_OR + 1;
const int WHERE_LT = WHERE_EQ + 1;
const int WHERE_GT = WHERE_LT + 1;
const int WHERE_NE = WHERE_GT + 1;
const int WHERE_LE = WHERE_NE + 1;
const int WHERE_GE = WHERE_LE + 1;
const int WHERE_IS_NULL = WHERE_GE + 1;
const int WHERE_IS_NOT_NULL = WHERE_IS_NULL + 1;

const int EXPR_ADD = 4000;
const int EXPR_SUB = EXPR_ADD + 1;
const int EXPR_UMINUS = EXPR_SUB + 1;

#endif