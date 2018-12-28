#ifndef PARSER_AST
#define PARSER_AST

#include <cstring>
#include <iostream>
#include "parser/ast_base.h"

class AstKeyword : public AstBase {
    public:
        AstKeyword(int code, std::string name) : AstBase(AST_KEYWORD) {
            this->val = code;
            this->name = name;
        }
        virtual bool checkSemantic(SemRecorder& sm) const {
            return !(sm.isInField() && val == AST_NULL);
        }
        virtual void printTree(IdentPrinter& ip) const {
            ip.writeln("keyword: " + name);
        }
        std::string toString() const {
            return name;
        }
        virtual std::any eval() final {
            return this->toString();
        }

    public:
        std::string name;
};

class AstOper : public AstBase {
    public:
        AstOper(int code) : AstBase(AST_OPER) {
            this->val = code;
        }
        AstOper(int code, const std::string& str) {
            new (this) AstOper(code);
            expr = str;
        }
        virtual bool checkSemantic(SemRecorder& sm) const {
            return true;
        }
        virtual void printTree(IdentPrinter& ip) const {
            ip.writeln("oper: " + this->toString());
        }
        std::string toString() const {
            return expr.length() ? expr : "" + (char)val;
        }
        virtual std::any eval() final {
            return this->toString();
        }

    public:
        std::string expr;
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
        virtual bool checkSemantic(SemRecorder& sm) const {
            return !(sm.isInField() && literalType == L_STRING);
        }
        virtual void printTree(IdentPrinter& ip) const {
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
        std::string toString() const {
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
        virtual std::any eval() final {
            switch (literalType) {
                case L_INT:
                    return val;
                case L_DECIMAL:
                    return floatval;
                case L_STRING:
                    return std::string(strval);
            }
            return nullptr;
        }

    public:
        int literalType;
};

class AstIdentifier : public AstBase {
    public:
        AstIdentifier(const char* val) : AstBase(AST_IDENTIFIER) {
            int l = strlen(val);
            this->strval = new char[l + 5];
            strcpy(this->strval, val);
            for (int i = 0; i < l; ++i)
                if (this->strval[i] >= 'A' && this->strval[i] <= 'Z')
                    this->strval[i] += 'a' - 'A';
        }
        virtual bool checkSemantic(SemRecorder& sm) const {
            return true;
        }
        virtual void printTree(IdentPrinter& ip) const {
            ip.writeln("identifier: " + std::string(strval));
        }
        std::string toString() const {
            return std::string(this->strval);
        }
        virtual std::any eval() final {
            return this->toString();
        }
};

class AstTopLevel : public AstBase {
    public:
        AstTopLevel(AstBase* stmtList) : AstBase(AST_TOPLEVEL) {
            this->stmtList = stmtList;
        }
        virtual bool checkSemantic(SemRecorder& sm) const {
            return stmtList->checkSemantic(sm);
        }
        virtual void printTree(IdentPrinter& ip) const {
            stmtList->printTree(ip);
        }
        virtual std::any eval() final;

    public:
        AstBase* stmtList;
};

class AstStmtList : public AstBase {
    public:
        AstStmtList(AstBase* ex, AstBase* rightmost) : AstBase(AST_STMTLIST) {
            if (ex != NULL)
                this->stmtList = dynamic_cast<AstStmtList*>(ex)->stmtList;
            this->stmtList.push_back(rightmost);
        }
        virtual bool checkSemantic(SemRecorder& sm) const {
            for (auto f : stmtList)
                if (!f->checkSemantic(sm))
                    return false;
            return true;
        }
        virtual void printTree(IdentPrinter& ip) const {
            ip.writeln("program");
            ip.ident();
            for (auto f : stmtList)
                f->printTree(ip);
            ip.deident();
        }
        virtual std::any eval() final;
    
    public:
        std::vector<AstBase*> stmtList;
};

class AstShowDB : public AstBase {
    public:
        AstShowDB() : AstBase(AST_SHOWDB) { }
        virtual bool checkSemantic(SemRecorder& sm) const {
            return true;
        }
        virtual void printTree(IdentPrinter& ip) const {
            ip.writeln("show databases");
        }
        virtual std::any eval() final;
};

class AstSetParam : public AstBase {
    public:
        AstSetParam(AstBase* param, AstBase* val) : AstBase(AST_SETPARAM) {
            this->param = param;
            this->pval = val;
        }
        virtual bool checkSemantic(SemRecorder& sm) const {
            // now we do not set any system params
            return false;
        }
        virtual void printTree(IdentPrinter& ip) const {
            ip.writeln("set system parameter");
            ip.ident();
            param->printTree(ip);
            pval->printTree(ip);
            ip.deident();
        }
    
    public:
        AstBase* param;
        AstBase* pval;
};

class AstCreateDB : public AstBase {
    public:
        AstCreateDB(AstBase* name) : AstBase(AST_CREATEDB) {
            this->name = name;
        }
        virtual bool checkSemantic(SemRecorder& sm) const {
            return true;
        }
        virtual void printTree(IdentPrinter& ip) const {
            ip.writeln("create database");
            ip.ident();
            name->printTree(ip);
            ip.deident();
        }
        virtual std::any eval() final;
    
    public:
        AstBase* name;
};

class AstDropDB : public AstBase {
    public:
        AstDropDB(AstBase* name) : AstBase(AST_DROPDB) {
            this->name = name;
        }
        virtual bool checkSemantic(SemRecorder& sm) const {
            return true;
        }
        virtual void printTree(IdentPrinter& ip) const {
            ip.writeln("drop database");
            ip.ident();
            name->printTree(ip);
            ip.deident();
        }
        virtual std::any eval() final;
    
    public:
        AstBase* name;
};

class AstUseDB : public AstBase {
    public:
        AstUseDB(AstBase* name) : AstBase(AST_USEDB) {
            this->name = name;
        }
        virtual bool checkSemantic(SemRecorder& sm) const {
            return true;
        }
        virtual void printTree(IdentPrinter& ip) const {
            ip.writeln("use database");
            ip.ident();
            name->printTree(ip);
            ip.deident();
        }
        virtual std::any eval() final;
    
    public:
        AstBase* name;
};

class AstShowTables : public AstBase {
    public:
        AstShowTables() : AstBase(AST_SHOWTABLES) { }
        virtual bool checkSemantic(SemRecorder& sm) const {
            return true;
        }
        virtual void printTree(IdentPrinter& ip) const {
            ip.writeln("show tables");
        }
        virtual std::any eval() final;
};

class AstCreateTable : public AstBase {
    public:
        AstCreateTable(AstBase* name, AstBase* fieldList) : AstBase(AST_CREATETABLE) {
            this->name = name;
            this->fieldList = fieldList;
        }
        virtual bool checkSemantic(SemRecorder& sm) const {
            return fieldList->checkSemantic(sm);
        }
        virtual void printTree(IdentPrinter& ip) const {
            ip.writeln("create table");
            ip.ident();
            name->printTree(ip);
            fieldList->printTree(ip);
            ip.deident();
        }
        virtual std::any eval() final;

    public:
        AstBase* name;
        AstBase* fieldList;
};

class AstDropTable : public AstBase {
    public:
        AstDropTable(AstBase* name) : AstBase(AST_DROPTABLE) {
            this->name = name;
        }
        virtual bool checkSemantic(SemRecorder& sm) const {
            return true;
        }
        virtual void printTree(IdentPrinter& ip) const {
            ip.writeln("drop table");
            ip.ident();
            name->printTree(ip);
            ip.deident();
        }
        virtual std::any eval() final;
    
    public:
        AstBase* name;
};

class AstDesc : public AstBase {
    public:
        AstDesc(AstBase* name) : AstBase(AST_DESC) {
            this->name = name;
        }
        virtual bool checkSemantic(SemRecorder& sm) const {
            return true;
        }
        virtual void printTree(IdentPrinter& ip) const {
            ip.writeln("describe table");
            ip.ident();
            name->printTree(ip);
            ip.deident();
        }
        virtual std::any eval() final;
    
    public:
        AstBase* name;
};

class AstInsert : public AstBase {
    public:
        AstInsert(AstBase* table, AstBase* valList) : AstBase(AST_INSERT) {
            this->table = table;
            this->valList = valList;
        }
        virtual bool checkSemantic(SemRecorder& sm) const {
            return valList->checkSemantic(sm);
        }
        virtual void printTree(IdentPrinter& ip) const {
            ip.writeln("insert");
            ip.ident();
            table->printTree(ip);
            valList->printTree(ip);
            ip.deident();
        }
        virtual std::any eval() final;

    public:
        AstBase* table;
        AstBase* valList;
};

class AstDelete : public AstBase {
    public:
        AstDelete(AstBase* table, AstBase* whereClause) : AstBase(AST_DELETE) {
            this->table = table;
            this->whereClause = whereClause;
        }
        virtual bool checkSemantic(SemRecorder& sm) const {
            return whereClause == NULL || whereClause->checkSemantic(sm);
        }
        virtual void printTree(IdentPrinter& ip) const {
            ip.writeln("delete");
            ip.ident();
            table->printTree(ip);
            if (whereClause != NULL)
                whereClause->printTree(ip);
            else
                ip.writeln("(all)");
            ip.deident();
        }

    public:
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
        virtual bool checkSemantic(SemRecorder& sm) const {
            return setClause->checkSemantic(sm) && (whereClause == NULL || whereClause->checkSemantic(sm));
        }
        virtual void printTree(IdentPrinter& ip) const {
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

    public:
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
        virtual bool checkSemantic(SemRecorder& sm) const {
            return whereClause == NULL || whereClause->checkSemantic(sm);
        }
        virtual void printTree(IdentPrinter& ip) const {
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
    
    public:
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
        virtual bool checkSemantic(SemRecorder& sm) const {
            return true;
        }
        virtual void printTree(IdentPrinter& ip) const { 
            ip.writeln("create index");
            ip.ident();
            table->printTree(ip);
            colName->printTree(ip);
            ip.deident();
        }
        virtual std::any eval() final;

    public:
        AstBase* table;
        AstBase* colName;
};

class AstDropIndex : public AstBase {
    public:
        AstDropIndex(AstBase* table, AstBase* colName) : AstBase(AST_DROPINDEX) {
            this->table = table;
            this->colName = colName;
        }
        virtual bool checkSemantic(SemRecorder& sm) const {
            return true;
        }
        virtual void printTree(IdentPrinter& ip) const { 
            ip.writeln("drop index");
            ip.ident();
            table->printTree(ip);
            colName->printTree(ip);
            ip.deident();
        }
        virtual std::any eval() final;

    public:
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
        virtual bool checkSemantic(SemRecorder& sm) const {
            sm.beginField();
            for (auto f : fieldList)
                if (!f->checkSemantic(sm))
                    return false;
            sm.endField();
            return true;
        }
        virtual void printTree(IdentPrinter& ip) const {
            ip.writeln("field list");
            ip.ident();
            for (auto f : fieldList)
                f->printTree(ip);
            ip.deident();
        }
    
    public:
        std::vector<AstBase*> fieldList;
};

class AstField : public AstBase {
    public:
        AstField(AstBase* name, AstBase* type, bool isNotNull) : AstBase(AST_FIELD) {
            this->name = name;
            this->ftype = type;
            this->isNotNull = isNotNull;
        }
        virtual bool checkSemantic(SemRecorder& sm) const {
            if (!sm.push(dynamic_cast<AstIdentifier*>(name)->toString()))
                return false;
            return ftype->checkSemantic(sm);
        }
        virtual void printTree(IdentPrinter& ip) const {
            ip.writeln("field");
            ip.ident();
            name->printTree(ip);
            ftype->printTree(ip);
            if (isNotNull)
                ip.writeln("not null");
            ip.deident();
        }
    
    public:
        AstBase* name;
        AstBase* ftype;
        bool isNotNull;
};

class AstPrimaryKeyDecl : public AstBase {
    public:
        AstPrimaryKeyDecl(AstBase* colList) : AstBase(AST_PRIMKEYDECL) {
            this->colList = colList;
        }
        virtual bool checkSemantic(SemRecorder& sm) const {
            return colList->checkSemantic(sm);
        }
        virtual void printTree(IdentPrinter& ip) const {
            ip.writeln("primary key declaration");
            ip.ident();
            colList->printTree(ip);
            ip.deident();
        }
    
    public:
        AstBase* colList;
};

class AstForeignKeyDecl : public AstBase {
    public:
        AstForeignKeyDecl(AstBase* colName, AstBase* ref, AstBase* refColName) : AstBase(AST_FOREKEYDECL) {
            this->colName = colName;
            this->ref = ref;
            this->refColName = refColName;
        }
        virtual bool checkSemantic(SemRecorder& sm) const {
            return sm.check(dynamic_cast<AstIdentifier*>(colName)->toString());
        }
        virtual void printTree(IdentPrinter& ip) const {
            ip.writeln("foreign key declaration");
            ip.ident();
            colName->printTree(ip);
            ref->printTree(ip);
            refColName->printTree(ip);
            ip.deident();
        }

    public:
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
        virtual bool checkSemantic(SemRecorder& sm) const {
            int v;
            if (len != NULL) {
                AstLiteral* ptr = dynamic_cast<AstLiteral*>(len);
                if (ptr->literalType != L_INT)
                    return false;
                v = ptr->val;
            }
            switch (val) {
                case TYPE_INT:
                    if (v < 1 || v > 10) {
                        std::cout << "int size should be in 1~11 instead of " << v << std::endl;
                        return false;
                    }
                    return true;
                case TYPE_CHAR:
                case TYPE_VARCHAR:
                    if (v < 1 || v > 255) {
                        std::cout << "string size should be in 1~255 instead of " << v << std::endl;
                        return false;
                    }
                    return true;
                case TYPE_FLOAT:
                case TYPE_DATE:
                    return true;
            }
            return false;
        }
        virtual void printTree(IdentPrinter& ip) const {
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

    public:
        AstBase* len;
};

class AstValLists : public AstBase {
    public:
        AstValLists(AstBase* ex, AstBase* rightmost) : AstBase(AST_VALLISTS) {
            if (ex != NULL)
                this->valLists = dynamic_cast<AstValLists*>(ex)->valLists;
            this->valLists.push_back(rightmost);
        }
        virtual bool checkSemantic(SemRecorder& sm) const {
            return true;
        }
        virtual void printTree(IdentPrinter& ip) const {
            ip.writeln("value lists");
            ip.ident();
            for (auto f : valLists)
                f->printTree(ip);
            ip.deident();
        }
    
    public:
        std::vector<AstBase*> valLists;
};

class AstValList : public AstBase {
    public:
        AstValList(AstBase* ex, AstBase* rightmost) : AstBase(AST_VALLIST) {
            if (ex != NULL)
                this->valList = dynamic_cast<AstValList*>(ex)->valList;
            this->valList.push_back(rightmost);
        }
        virtual bool checkSemantic(SemRecorder& sm) const {
            return true;
        }
        virtual void printTree(IdentPrinter& ip) const {
            ip.writeln("vallist");
            ip.ident();
            for (auto f : valList)
                f->printTree(ip);
            ip.deident();
        }
    
    public:
        std::vector<AstBase*> valList;
};

class AstWhereClause : public AstBase {
    public:
        AstWhereClause(AstBase* lhs, AstBase* rhs, int opId) : AstBase(AST_WHERECLAUSE) {
            this->lhs = lhs;
            this->rhs = rhs;
            this->val = opId;
        }
        virtual bool checkSemantic(SemRecorder& sm) const {
            return true;
        }
        virtual void printTree(IdentPrinter& ip) const {
            switch (val) {
                case WHERE_AND:
                    ip.writeln("where and of");
                    break;
                case WHERE_OR:
                    ip.writeln("where or of");
                    break;
                case WHERE_NOT:
                    ip.writeln("where not");
                    break;
                case WHERE_EQ:
                    ip.writeln("where =");
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

    public:
        AstBase* lhs;
        AstBase* rhs;
};

class AstCol : public AstBase {
    public:
        AstCol(AstBase* owner, AstBase* colName) : AstBase(AST_COL) {
            this->owner = owner;
            this->colName = colName;
        }
        virtual bool checkSemantic(SemRecorder& sm) const {
            return true;
        }
        virtual void printTree(IdentPrinter& ip) const {
            ip.writeln("column");
            ip.ident();
            if (owner != NULL)
                owner->printTree(ip);
            else
                ip.writeln("(self)");
            colName->printTree(ip);
            ip.deident();
        }

    public:
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
        virtual bool checkSemantic(SemRecorder& sm) const {
            sm.beginExpr();
            bool res = lhs->checkSemantic(sm) && (rhs == NULL || rhs->checkSemantic(sm));
            sm.endExpr();
            return res;
        }
        virtual void printTree(IdentPrinter& ip) const {
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

    public:
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
        virtual bool checkSemantic(SemRecorder& sm) const {
            return true;
        }
        virtual void printTree(IdentPrinter& ip) const {
            ip.writeln("set clause");
            ip.ident();
            for (auto f : setList)
                f->printTree(ip);
            ip.deident();
        }
    
    public:
        std::vector<AstBase*> setList;
};

class AstSet : public AstBase {
    public:
        AstSet(AstBase* colName, AstBase* val) : AstBase(AST_SET) {
            this->colName = colName;
            this->sval = val;
        }
        virtual bool checkSemantic(SemRecorder& sm) const {
            return true;
        }
        virtual void printTree(IdentPrinter& ip) const {
            ip.writeln("set");
            ip.ident();
            colName->printTree(ip);
            sval->printTree(ip);
            ip.deident();
        }
    
    public:
        AstBase* colName;
        AstBase* sval;
};

class AstSelector : public AstBase {
    public:
        AstSelector(AstBase* colList) : AstBase(AST_SELECTOR) {
            this->colList = colList;
        }
        virtual bool checkSemantic(SemRecorder& sm) const {
            return true;
        }
        virtual void printTree(IdentPrinter& ip) const {
            ip.writeln("selector");
            ip.ident();
            if (colList != NULL)
                colList->printTree(ip);
            else
                ip.writeln("*");
            ip.deident();
        }

    public:
        AstBase* colList;
};

class AstColList : public AstBase {
    public:
        AstColList(AstBase* ex, AstBase* rightmost) : AstBase(AST_COLLIST) {
            if (ex != NULL)
                this->colList = dynamic_cast<AstColList*>(ex)->colList;
            this->colList.push_back(rightmost);
        }
        virtual bool checkSemantic(SemRecorder& sm) const {
            for (auto f : colList)
                if (!f->checkSemantic(sm))
                    return false;
            return true;
        }
        virtual void printTree(IdentPrinter& ip) const {
            ip.writeln("column list");
            ip.ident();
            for (auto f : colList)
                f->printTree(ip);
            ip.deident();
        }
    
    public:
        std::vector<AstBase*> colList;
};

class AstIdentList : public AstBase {
    public:
        AstIdentList(AstBase* ex, AstBase* rightmost) : AstBase(AST_IDENTLIST) {
            if (ex != NULL)
                this->identList = dynamic_cast<AstIdentList*>(ex)->identList;
            this->identList.push_back(rightmost);
        }
        virtual bool checkSemantic(SemRecorder& sm) const {
            if (sm.isInField())
                for (auto f : identList) {
                    std::string ident = dynamic_cast<AstIdentifier*>(f)->toString();
                    if (!sm.check(ident))
                        return false;
                }
            return true;
        }
        virtual void printTree(IdentPrinter& ip) const {
            ip.writeln("identifier list");
            ip.ident();
            for (auto f : identList)
                f->printTree(ip);
            ip.deident();
        }
    
    public:
        std::vector<AstBase*> identList;
};

#endif