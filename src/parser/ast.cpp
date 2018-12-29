#include <iostream>

#include "global.h"
#include "parser/ast.h"

using namespace std;

extern Global* global;

any AstTopLevel::eval() {
    return this->stmtList->eval();
}

any AstStmtList::eval() {
    for (auto s : this->stmtList) {
        try {
            if (!any_cast<bool>(s->eval()))
                throw EvalException("error");
        }
        catch (EvalException ex) {
            cout << ex.what() << endl;
            return false;
        }
    }
    return true;
}

any AstSetParam::eval() {
    string par = dynamic_cast<AstIdentifier*>(param)->toString();
    string ret;
    if (pval->type == AST_NULL)
        ret = "null";
    else
        ret = dynamic_cast<AstLiteral*>(pval)->toString();
    global->settings[par] = ret;
    return true;
}

any AstShowDB::eval() {
    bool res = global->mm->evalAst(this);
    cout << "Databases:" << endl;
    for (auto db : global->strList)
        cout << "    " << db << endl;
    return res;
}

any AstCreateDB::eval() {
    bool res = global->mm->evalAst(this);
    string name = dynamic_cast<AstIdentifier*>(this->name)->toString();
    if (res)
        cout << "Create database " << name << endl;
    else
        throw EvalException(name + " already exists");
    return res;
}

any AstDropDB::eval() {
    bool res = global->mm->evalAst(this);
    string name = dynamic_cast<AstIdentifier*>(this->name)->toString();
    if (res)
        cout << "Drop database " << name << endl;
    else
        throw EvalException(name + " does not exist");
    return res;
}

any AstUseDB::eval() {
    bool res = global->mm->evalAst(this);
    string name = dynamic_cast<AstIdentifier*>(this->name)->toString();
    if (res)
        cout << "Switch to " << name << endl;
    else
        throw EvalException(name + " does not exist");
    return res;
}

any AstShowTables::eval() {
    bool res = global->mm->evalAst(this);
    if (res) {
        cout << "Tables:" << endl;
        for (auto tb : global->strList)
            cout << "    " << tb << endl;
    }
    else
        throw EvalException("not using any database now");
    return res;
}

any AstCreateTable::eval() {
    bool res = global->mm->evalAst(this);
    string name = dynamic_cast<AstIdentifier*>(this->name)->toString();
    if (res)
        cout << "Create table " << name << endl;
    else
        throw EvalException("cannot create table " + name + " in current context");
    return res;
}

any AstDropTable::eval() {
    bool res = global->mm->evalAst(this);
    string name = dynamic_cast<AstIdentifier*>(this->name)->toString();
    if (res)
        cout << "Delete table " << name << endl;
    else
        throw EvalException("cannot delete table " + name + " in current context");
    return res;
}

any AstDesc::eval() {
    bool res = global->mm->evalAst(this);
    string name = dynamic_cast<AstIdentifier*>(this->name)->toString();
    if (!res)
        throw EvalException("cannot describe table" + name + " in current context");
    return res;
}

any AstCreateIndex::eval() {
    bool res = global->mm->evalAst(this);
    string colName = dynamic_cast<AstIdentifier*>(this->colName)->toString();
    string table = dynamic_cast<AstIdentifier*>(this->table)->toString();
    if (res)
        cout << "Create index on " << colName << " of table " << table << endl;
    else
        throw EvalException("cannot create index on " + colName + " of table " + table + " in current context");
    return res;
}

any AstDropIndex::eval() {
    bool res = global->mm->evalAst(this);
    string colName = dynamic_cast<AstIdentifier*>(this->colName)->toString();
    string table = dynamic_cast<AstIdentifier*>(this->table)->toString();
    if (res)
        cout << "Drop index on " << colName << " of table " << table << endl;
    else
        throw EvalException("cannot delete index on " + colName + " of table " + table + " in current context");
    return res;
}

any AstInsert::eval() {
    bool res = global->ql->evalAst(this);
    string tablename = dynamic_cast<AstIdentifier*>(this->table)->toString();
    if (res)
        cout << "Insert value into table " << tablename << endl;
    else
        throw EvalException("cannot insert value into table " + tablename + " in current context");
    return res;
}

any AstDelete::eval() {
    bool res = global->ql->evalAst(this);
    string tablename = dynamic_cast<AstIdentifier*>(this->table)->toString();
    if (res)
        cout << "Delete value from table " << tablename << endl;
    else
        throw EvalException("Fail: cannot delete value from table " + tablename + " in current context");
    return res;
}

any AstUpdate::eval() {
    bool res = global->ql->evalAst(this);
    string tablename = dynamic_cast<AstIdentifier*>(this->table)->toString();
    if (res)
        cout << "Update value in table " << tablename << endl;
    else
        throw EvalException("Fail: cannot update value in table " + tablename + " in current context");
    return res;
}

any AstSelect::eval() {
    bool res = global->ql->evalAst(this);
    if (res)
        cout << "Select value succeed"  << endl;
    else
        throw EvalException("Fail: cannot select value in current context");
    return res;
}