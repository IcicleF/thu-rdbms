#ifndef PARSER_AST
#define PARSER_AST

#include <cstring>
#include <iostream>
#include "ast_base.h"

class AstKeyword : public AstBase {
    public:
        AstKeyword(int code, std::string name) : AstBase(AST_KEYWORD) {
            this->val = code;
            this->name = name;
        }
        virtual void printTree(IdentPrinter& ip) {
            ip.writeln("keyword: " + name);
        }
        std::string toString() {
            return name;
        }

    protected:
        std::string name;
};

class AstOper : public AstBase {
    public:
        AstOper(int code) : AstBase(AST_OPER) {
            this->val = code;
        }
        virtual void printTree(IdentPrinter& ip) {
            ip.writeln("oper: " + (char)val);
        }
        std::string toString() {
            return "" + (char)val;
        }
};

class AstLiteral : public AstBase {
    public:
        AstLiteral(const char* val, int type) : AstBase(AST_LITERAL) {
            int len = strlen(val);
            this->literalType = type;
            switch (type) {
                case L_INT:
                    this->val = atoi(val);
                    break;
                case L_DECIMAL:
                    this->floatval = (float)atof(val);
                    break;
                case L_STRING:
                    this->strval = new char[len + 5];
                    strcpy(this->strval, val);
                    break;
                default:
                    break;
            }
        }
        virtual void printTree(IdentPrinter& ip) {
            std::string s = "literal ";
            switch (literalType) {
                case L_INT:
                    s += "(int): " + std::to_string(val);
                    break;
                case L_DECIMAL:
                    s += "(float): " + std::to_string(floatval);
                    break;
                case L_STRING:
                    s += "(string): " + std::string(strval);
                    break;
                default:
                    s += "(unknown)";
            }
            ip.writeln(s);
        }
        std::string toString() {
            switch (literalType) {
                case L_INT:
                    return std::to_string(val);
                case L_DECIMAL:
                    return std::to_string(floatval);
                case L_STRING:
                    return std::string(strval);
            }
            return "unknown";
        }

    protected:
        int literalType;
};

class AstIdentifier : public AstBase {
    public:
        AstIdentifier(const char* val) : AstBase(AST_IDENTIFIER) {
            this->strval = new char[strlen(val) + 5];
            strcpy(this->strval, val);
        }
        virtual void printTree(IdentPrinter& ip) {
            ip.writeln("identifier: " + std::string(strval));
        }
};

class AstTopLevel : public AstBase {
    public:
        AstTopLevel(AstBase* stmtList) : AstBase(AST_TOPLEVEL) {
            this->stmtList = stmtList;
        }
        virtual void printTree(IdentPrinter& ip) {
            stmtList->printTree(ip);
        }

    protected:
        AstBase* stmtList;
};

class AstStmtList : public AstBase {
    public:
        AstStmtList(AstBase* ex, AstBase* rightmost) : AstBase(AST_STMTLIST) {
            if (ex != NULL)
                this->stmtList = dynamic_cast<AstStmtList*>(ex)->stmtList;
            this->stmtList.push_back(rightmost);
        }
        virtual void printTree(IdentPrinter& ip) {
            ip.writeln("program");
            ip.ident();
            for (auto f : stmtList)
                f->printTree(ip);
            ip.deident();
        }
    
    protected:
        std::vector<AstBase*> stmtList;
};

class AstShowDB : public AstBase {
    public:
        AstShowDB() : AstBase(AST_SHOWDB) { }
        virtual void printTree(IdentPrinter& ip) {
            ip.writeln("show databases");
        }
};

class AstSetParam : public AstBase {
    public:
        AstSetParam(AstBase* param, AstBase* val) : AstBase(AST_SETPARAM) {
            this->param = param;
            this->pval = val;
        }
        virtual void printTree(IdentPrinter& ip) {
            ip.writeln("set system parameter");
            ip.ident();
            param->printTree(ip);
            pval->printTree(ip);
            ip.deident();
        }
    
    protected:
        AstBase* param;
        AstBase* pval;
};

class AstCreateDB : public AstBase {
    public:
        AstCreateDB(AstBase* name) : AstBase(AST_CREATEDB) {
            this->name = name;
        }
        virtual void printTree(IdentPrinter& ip) {
            ip.writeln("create database");
            ip.ident();
            name->printTree(ip);
            ip.deident();
        }
    
    protected:
        AstBase* name;
};

class AstDropDB : public AstBase {
    public:
        AstDropDB(AstBase* name) : AstBase(AST_DROPDB) {
            this->name = name;
        }
        virtual void printTree(IdentPrinter& ip) {
            ip.writeln("drop database");
            ip.ident();
            name->printTree(ip);
            ip.deident();
        }
    
    protected:
        AstBase* name;
};

class AstUseDB : public AstBase {
    public:
        AstUseDB(AstBase* name) : AstBase(AST_USEDB) {
            this->name = name;
        }
        virtual void printTree(IdentPrinter& ip) {
            ip.writeln("use database");
            ip.ident();
            name->printTree(ip);
            ip.deident();
        }
    
    protected:
        AstBase* name;
};

class AstShowTables : public AstBase {
    public:
        AstShowTables() : AstBase(AST_SHOWTABLES) { }
        virtual void printTree(IdentPrinter& ip) {
            ip.writeln("show tables");
        }
};

class AstCreateTable : public AstBase {
    public:
        AstCreateTable(AstBase* name, AstBase* fieldList) : AstBase(AST_CREATETABLE) {
            this->name = name;
            this->fieldList = fieldList;
        }
        virtual void printTree(IdentPrinter& ip) {
            ip.writeln("create table");
            ip.ident();
            name->printTree(ip);
            fieldList->printTree(ip);
            ip.deident();
        }

    protected:
        AstBase* name;
        AstBase* fieldList;
};

class AstDropTable : public AstBase {
    public:
        AstDropTable(AstBase* name) : AstBase(AST_DROPTABLE) {
            this->name = name;
        }
        virtual void printTree(IdentPrinter& ip) {
            ip.writeln("drop table");
            ip.ident();
            name->printTree(ip);
            ip.deident();
        }
    
    protected:
        AstBase* name;
};

class AstDesc : public AstBase {
    public:
        AstDesc(AstBase* name) : AstBase(AST_DESC) {
            this->name = name;
        }
        virtual void printTree(IdentPrinter& ip) {
            ip.writeln("describe table");
            ip.ident();
            name->printTree(ip);
            ip.deident();
        }
    
    protected:
        AstBase* name;
};

class AstInsert : public AstBase {
    public:
        AstInsert(AstBase* table, AstBase* valList) : AstBase(AST_INSERT) {
            this->table = table;
            this->valList = valList;
        }
        virtual void printTree(IdentPrinter& ip) {
            ip.writeln("insert");
            ip.ident();
            table->printTree(ip);
            valList->printTree(ip);
            ip.deident();
        }

    protected:
        AstBase* table;
        AstBase* valList;
};

class AstDelete : public AstBase {
    public:
        AstDelete(AstBase* table, AstBase* whereClause) : AstBase(AST_DELETE) {
            this->table = table;
            this->whereClause = whereClause;
        }
        virtual void printTree(IdentPrinter& ip) {
            ip.writeln("delete");
            ip.ident();
            table->printTree(ip);
            if (whereClause != NULL)
                whereClause->printTree(ip);
            else
                ip.writeln("(all)");
            ip.deident();
        }

    protected:
        AstBase* table;
        AstBase* whereClause;
};

class AstUpdate : public AstBase {
    public:
        AstUpdate(AstBase* table, AstBase* setClause, AstBase* whereClause) : AstBase(AST_UPDATE) {
            this->table = table;
            this->setClause = setClause;
            this->whereClause = whereClause;
        }
        virtual void printTree(IdentPrinter& ip) {
            ip.writeln("update");
            ip.ident();
            table->printTree(ip);
            setClause->printTree(ip);
            if (whereClause != NULL)
                whereClause->printTree(ip);
            else
                ip.writeln("(all)");
            ip.deident();
        }

    protected:
        AstBase* table;
        AstBase* setClause;
        AstBase* whereClause;
};

class AstSelect : public AstBase {
    public:
        AstSelect(AstBase* selector, AstBase* tableList, AstBase* whereClause) : AstBase(AST_SELECT) {
            this->selector = selector;
            this->tableList = tableList;
            this->whereClause = whereClause;
        }
        virtual void printTree(IdentPrinter& ip) {
            ip.writeln("select");
            ip.ident();
            selector->printTree(ip);
            tableList->printTree(ip);
            if (whereClause != NULL)
                whereClause->printTree(ip);
            else
                ip.writeln("(all)");
            ip.deident();
        }
    
    protected:
        AstBase* selector;
        AstBase* tableList;
        AstBase* whereClause;
};

class AstCreateIndex : public AstBase {
    public:
        AstCreateIndex(AstBase* table, AstBase* colName) : AstBase(AST_CREATEINDEX) {
            this->table = table;
            this->colName = colName;
        }
        virtual void printTree(IdentPrinter& ip) { 
            ip.writeln("create index");
            ip.ident();
            table->printTree(ip);
            colName->printTree(ip);
            ip.deident();
        }

    protected:
        AstBase* table;
        AstBase* colName;
};

class AstDropIndex : public AstBase {
    public:
        AstDropIndex(AstBase* table, AstBase* colName) : AstBase(AST_DROPINDEX) {
            this->table = table;
            this->colName = colName;
        }
        virtual void printTree(IdentPrinter& ip) { 
            ip.writeln("drop index");
            ip.ident();
            table->printTree(ip);
            colName->printTree(ip);
            ip.deident();
        }

    protected:
        AstBase* table;
        AstBase* colName;
};

class AstFieldList : public AstBase {
    public:
        AstFieldList(AstBase* ex, AstBase* rightmost) : AstBase(AST_FIELDLIST) {
            if (ex != NULL)
                this->fieldList = dynamic_cast<AstFieldList*>(ex)->fieldList;
            this->fieldList.push_back(rightmost);
        }
        virtual void printTree(IdentPrinter& ip) {
            ip.writeln("field list");
            ip.ident();
            for (auto f : fieldList)
                f->printTree(ip);
            ip.deident();
        }
    
    protected:
        std::vector<AstBase*> fieldList;
};

class AstField : public AstBase {
    public:
        AstField(AstBase* name, AstBase* type, bool isNotNull) {
            this->name = name;
            this->ftype = type;
            this->isNotNull = isNotNull;
        }
        virtual void printTree(IdentPrinter& ip) {
            ip.writeln("field");
            ip.ident();
            name->printTree(ip);
            ftype->printTree(ip);
            if (isNotNull)
                ip.writeln("not null");
            ip.deident();
        }
    
    protected:
        AstBase* name;
        AstBase* ftype;
        bool isNotNull;
};

class AstPrimaryKeyDecl : public AstBase {
    public:
        AstPrimaryKeyDecl(AstBase* colList) : AstBase(AST_PRIMKEYDECL) {
            this->colList = colList;
        }
        virtual void printTree(IdentPrinter& ip) {
            ip.writeln("primary key declaration");
            ip.ident();
            colList->printTree(ip);
            ip.deident();
        }
    
    protected:
        AstBase* colList;
};

class AstForeignKeyDecl : public AstBase {
    public:
        AstForeignKeyDecl(AstBase* colName, AstBase* ref, AstBase* refColName) : AstBase(AST_FOREKEYDECL) {
            this->colName = colName;
            this->ref = ref;
            this->refColName = refColName;
        }
        virtual void printTree(IdentPrinter& ip) {
            ip.writeln("foreign key declaration");
            ip.ident();
            colName->printTree(ip);
            ref->printTree(ip);
            refColName->printTree(ip);
            ip.deident();
        }

    protected:
        AstBase* colName;
        AstBase* ref;
        AstBase* refColName;
};

class AstType : public AstBase {
    public:
        AstType(int typeId, AstBase* len) : AstBase(AST_TYPE) {
            this->val = typeId;
            this->len = len;
        }
        virtual void printTree(IdentPrinter& ip) {
            std::string s = "type: ", lens = (len != NULL ? dynamic_cast<AstLiteral*>(len)->toString() : "");
            switch (val) {
                case TYPE_INT:
                    s += "int(" + lens + ")";
                    break;
                case TYPE_FLOAT:
                    s += "float";
                    break;
                case TYPE_CHAR:
                    s += "char(" + lens + ")";
                    break;
                case TYPE_VARCHAR:
                    s += "varchar(" + lens + ")";
                    break;
                case TYPE_DATE:
                    s += "date";
                    break;
                default:
                    s += "unknown";
            }
            ip.writeln(s);
        }

    protected:
        AstBase* len;
};

class AstValLists : public AstBase {
    public:
        AstValLists(AstBase* ex, AstBase* rightmost) : AstBase(AST_VALLISTS) {
            if (ex != NULL)
                this->valLists = dynamic_cast<AstValLists*>(ex)->valLists;
            this->valLists.push_back(rightmost);
        }
        virtual void printTree(IdentPrinter& ip) {
            ip.writeln("value lists");
            ip.ident();
            for (auto f : valLists)
                f->printTree(ip);
            ip.deident();
        }
    
    protected:
        std::vector<AstBase*> valLists;
};

class AstValList : public AstBase {
    public:
        AstValList(AstBase* ex, AstBase* rightmost) : AstBase(AST_VALLIST) {
            if (ex != NULL)
                this->valList = dynamic_cast<AstValList*>(ex)->valList;
            this->valList.push_back(rightmost);
        }
        virtual void printTree(IdentPrinter& ip) {
            ip.writeln("vallist");
            ip.ident();
            for (auto f : valList)
                f->printTree(ip);
            ip.deident();
        }
    
    protected:
        std::vector<AstBase*> valList;
};

class AstWhereClause : public AstBase {
    public:
        AstWhereClause(AstBase* lhs, AstBase* rhs, int opId) : AstBase(AST_WHERECLAUSE) {
            this->lhs = lhs;
            this->rhs = rhs;
            this->val = opId;
        }
        virtual void printTree(IdentPrinter& ip) {
            switch (val) {
                case WHERE_AND:
                    ip.writeln("where and of");
                    break;
                case WHERE_OR:
                    ip.writeln("where or of");
                    break;
                case WHERE_EQ:
                    ip.writeln("where ==");
                    break;
                case WHERE_GE:
                    ip.writeln("where >=");
                    break;
                case WHERE_GT:
                    ip.writeln("where >");
                    break;
                case WHERE_IS_NOT_NULL:
                    ip.writeln("where is not null");
                    break;
                case WHERE_IS_NULL:
                    ip.writeln("where is null");
                    break;
                case WHERE_LE:
                    ip.writeln("where <=");
                    break;
                case WHERE_LT:
                    ip.writeln("where <");
                    break;
                case WHERE_NE:
                    ip.writeln("where !=");
                    break;
                default:
                    ip.writeln("unknown where");
            }
            ip.ident();
            lhs->printTree(ip);
            if (rhs != NULL)
                rhs->printTree(ip);
            ip.deident();
        }

    protected:
        AstBase* lhs;
        AstBase* rhs;
};

class AstCol : public AstBase {
    public:
        AstCol(AstBase* owner, AstBase* colName) : AstBase(AST_COL) {
            this->owner = owner;
            this->colName = colName;
        }
        virtual void printTree(IdentPrinter& ip) {
            ip.writeln("column");
            ip.ident();
            if (owner != NULL)
                owner->printTree(ip);
            else
                ip.writeln("(self)");
            colName->printTree(ip);
            ip.deident();
        }

    protected:
        AstBase* owner;
        AstBase* colName;
};

class AstExpr : public AstBase {
    public:
        AstExpr(AstBase* lhs, AstBase* rhs, int opId) : AstBase(AST_EXPR) {
            this->lhs = lhs;
            this->rhs = rhs;
            this->val = opId;
        }
        virtual void printTree(IdentPrinter& ip) {
            switch (val) {
                case EXPR_ADD:
                    ip.writeln("add");
                    break;
                case EXPR_SUB:
                    ip.writeln("sub");
                    break;
                case EXPR_UMINUS:
                    ip.writeln("minus");
                    break;
                default:
                    ip.writeln("unknown expr");
            }
            ip.ident();
            lhs->printTree(ip);
            if (rhs != NULL)
                rhs->printTree(ip);
            ip.deident();
        }

    protected:
        AstBase* lhs;
        AstBase* rhs;
};

class AstSetClause : public AstBase {
    public:
        AstSetClause(AstBase* ex, AstBase* rightmost) : AstBase(AST_SETCLAUSE) {
            if (ex != NULL)
                this->setList = dynamic_cast<AstSetClause*>(ex)->setList;
            this->setList.push_back(rightmost);
        }
        virtual void printTree(IdentPrinter& ip) {
            ip.writeln("set clause");
            ip.ident();
            for (auto f : setList)
                f->printTree(ip);
            ip.deident();
        }
    
    protected:
        std::vector<AstBase*> setList;
};

class AstSet : public AstBase {
    public:
        AstSet(AstBase* colName, AstBase* val) : AstBase(AST_SET) {
            this->colName = colName;
            this->sval = val;
        }
        virtual void printTree(IdentPrinter& ip) {
            ip.writeln("set");
            ip.ident();
            colName->printTree(ip);
            sval->printTree(ip);
            ip.deident();
        }
    
    protected:
        AstBase* colName;
        AstBase* sval;
};

class AstSelector : public AstBase {
    public:
        AstSelector(AstBase* colList) : AstBase(AST_SELECTOR) {
            this->colList = colList;
        }
        virtual void printTree(IdentPrinter& ip) {
            ip.writeln("selector");
            ip.ident();
            if (colList != NULL)
                colList->printTree(ip);
            else
                ip.writeln("*");
            ip.deident();
        }

    private:
        AstBase* colList;
};

class AstColList : public AstBase {
    public:
        AstColList(AstBase* ex, AstBase* rightmost) : AstBase(AST_COLLIST) {
            if (ex != NULL)
                this->colList = dynamic_cast<AstColList*>(ex)->colList;
            this->colList.push_back(rightmost);
        }
        virtual void printTree(IdentPrinter& ip) {
            ip.writeln("column list");
            ip.ident();
            for (auto f : colList)
                f->printTree(ip);
            ip.deident();
        }
    
    protected:
        std::vector<AstBase*> colList;
};

class AstIdentList : public AstBase {
    public:
        AstIdentList(AstBase* ex, AstBase* rightmost) : AstBase(AST_IDENTLIST) {
            if (ex != NULL)
                this->identList = dynamic_cast<AstIdentList*>(ex)->identList;
            this->identList.push_back(rightmost);
        }
        virtual void printTree(IdentPrinter& ip) {
            ip.writeln("identifier list");
            ip.ident();
            for (auto f : identList)
                f->printTree(ip);
            ip.deident();
        }
    
    protected:
        std::vector<AstBase*> identList;
};

#endif