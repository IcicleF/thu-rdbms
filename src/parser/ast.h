#ifndef PARSER_AST
#define PARSER_AST

#include <cstring>
#include <iostream>
#include "ast_base.h"

class AstKeyword : public AstBase {
    public:
        AstKeyword(int code) : AstBase(AST_KEYWORD) {
            this->val = code;
        }
        virtual void printTree() { }
};

class AstLiteral : public AstBase {
    public:
        AstLiteral(char* val, int type) : AstBase(AST_LITERAL) {
            this->strval = new char[strlen(val) + 5];
            strcpy(this->strval, val);
        }
        virtual void printTree() { }
};

class AstIdentifier : public AstBase {
    public:
        AstIdentifier(char* val) : AstBase(AST_IDENTIFIER) {
            this->strval = new char[strlen(val) + 5];
            strcpy(this->strval, val);
        }
        virtual void printTree() { }
};

class AstTopLevel : public AstBase {
    public:
        AstTopLevel(AstBase* stmt) : AstBase(AST_TOPLEVEL) { 
            this->parent = NULL;
            this->childs.push_back(stmt);
        }
        virtual void printTree() { }
};

class AstShowDB : public AstBase {
    public:
        AstShowDB() : AstBase(AST_SHOWDB) {
            
        }
        virtual void printTree() { }
};

class AstSetParam : public AstBase {
    public:
        AstSetParam(AstBase* param, AstBase* val) : AstBase(AST_SETPARAM) {

        }
        virtual void printTree() { }
};

class AstCreateDB : public AstBase {
    public:
        AstCreateDB(AstBase* name) : AstBase(AST_CREATEDB) {

        }
        virtual void printTree() { }
};

class AstDropDB : public AstBase {
    public:
        AstDropDB(AstBase* name) : AstBase(AST_DROPDB) {

        }
        virtual void printTree() { }
};

class AstUseDB : public AstBase {
    public:
        AstUseDB(AstBase* name) : AstBase(AST_USEDB) {

        }
        virtual void printTree() { }
};

class AstShowTables : public AstBase {
    public:
        AstShowTables() : AstBase(AST_SHOWTABLES) {

        }
        virtual void printTree() { }
};

class AstCreateTable : public AstBase {
    public:
        AstCreateTable(AstBase* name, AstBase* fieldList) : AstBase(AST_CREATETABLE) {

        }
        virtual void printTree() { }
};

class AstDropTable : public AstBase {
    public:
        AstDropTable(AstBase* name) : AstBase(AST_DROPTABLE) {

        }
        virtual void printTree() { }
};

class AstDesc : public AstBase {
    public:
        AstDesc(AstBase* name) : AstBase(AST_DESC) {

        }
        virtual void printTree() { }
};

class AstInsert : public AstBase {
    public:
        AstInsert(AstBase* table, AstBase* valList) : AstBase(AST_INSERT) {

        }
        virtual void printTree() { }
};

class AstDelete : public AstBase {
    public:
        AstDelete(AstBase* table, AstBase* whereClause) : AstBase(AST_DELETE) {

        }
        virtual void printTree() { }
};

class AstUpdate : public AstBase {
    public:
        AstUpdate(AstBase* table, AstBase* setClause, AstBase* whereClause) : AstBase(AST_UPDATE) {

        }
        virtual void printTree() { }
};

class AstSelect : public AstBase {
    public:
        AstSelect(AstBase* selector, AstBase* tableList, AstBase* whereClause) : AstBase(AST_SELECT) {

        }
        virtual void printTree() { }
};

class AstCreateIndex : public AstBase {
    public:
        AstCreateIndex(AstBase* table, AstBase* colName) : AstBase(AST_CREATEINDEX) {

        }
        virtual void printTree() { }
};

class AstDropIndex : public AstBase {
    public:
        AstDropIndex(AstBase* table, AstBase* colName) : AstBase(AST_DROPINDEX) {

        }
        virtual void printTree() { }
};

class AstFieldList : public AstBase {
    public:
        AstFieldList(AstBase* ex, AstBase* rightmost) : AstBase(AST_FIELDLIST) {

        }
        virtual void printTree() { }
};

class AstField : public AstBase {
    public:
        AstField(AstBase* name, AstBase* type, bool isNotNull) {
            
        }
        virtual void printTree() { }
};

class AstPrimaryKeyDecl : public AstBase {
    public:
        AstPrimaryKeyDecl(AstBase* colList) : AstBase(AST_PRIMKEYDECL) {

        }
        virtual void printTree() { }
};

class AstForeignKeyDecl : public AstBase {
    public:
        AstForeignKeyDecl(AstBase* colName, AstBase* ref, AstBase* refColName) : AstBase(AST_FOREKEYDECL) {

        }
        virtual void printTree() { }
};

class AstType : public AstBase {
    public:
        AstType(int typeId, AstBase* len) : AstBase(AST_TYPE) {

        }
        virtual void printTree() { }
};

class AstValLists : public AstBase {
    public:
        AstValLists(AstBase* ex, AstBase* rightmost) : AstBase(AST_VALLISTS) {

        }
        virtual void printTree() { }
};

class AstValList : public AstBase {
    public:
        AstValList(AstBase* ex, AstBase* rightmost) : AstBase(AST_VALLIST) {

        }
        virtual void printTree() { }
};

class AstWhereClause : public AstBase {
    public:
        AstWhereClause(AstBase* lhs, AstBase* rhs, int opId) : AstBase(AST_WHERECLAUSE) {

        }
        virtual void printTree() { }
};

class AstCol : public AstBase {
    public:
        AstCol(AstBase* owner, AstBase* colName) : AstBase(AST_COL) {

        }
        virtual void printTree() { }
};

class AstExpr : public AstBase {
    public:
        AstExpr(AstBase* lhs, AstBase* rhs, int opId) : AstBase(AST_EXPR) {

        }
        virtual void printTree() { }
};

class AstSetClause : public AstBase {
    public:
        AstSetClause(AstBase* ex, AstBase* rightmost) : AstBase(AST_SETCLAUSE) {

        }
        virtual void printTree() { }
};

class AstSet : public AstBase {
    public:
        AstSet(AstBase* colName, AstBase* val) : AstBase(AST_SET) {

        }
        virtual void printTree() { }
};

class AstSelector : public AstBase {
    public:
        AstSelector(AstBase* colList) : AstBase(AST_SELECTOR) {

        }
        virtual void printTree() { }
};

class AstColList : public AstBase {
    public:
        AstColList(AstBase* ex, AstBase* rightmost) : AstBase(AST_COLLIST) {

        }
        virtual void printTree() { }
};

class AstIdentList : public AstBase {
    public:
        AstIdentList(AstBase* ex, AstBase* rightmost) : AstBase(AST_IDENTLIST) {

        }
        virtual void printTree() { }
};

#endif