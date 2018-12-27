#include <iostream>

#include "global.h"
#include "parser/ast.h"

using namespace std;

extern Global* global;

any AstTopLevel::eval() {
    try {
        this->stmtList->eval();
        return true;
    }
    catch (EvalException ex) {
        cout << endl << ex.what() << endl;
    }
    return false;
}

any AstStmtList::eval() {
    for (auto s : this->stmtList)
        s->eval();
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
        throw EvalException("Fail: " + name + " already exists");
    return res;
}

any AstDropDB::eval() {
    bool res = global->mm->evalAst(this);
    string name = dynamic_cast<AstIdentifier*>(this->name)->toString();
    if (res)
        cout << "Drop database " << name << endl;
    else
        throw EvalException("Fail: " + name + " does not exist");
    return res;
}

any AstUseDB::eval() {
    bool res = global->mm->evalAst(this);
    string name = dynamic_cast<AstIdentifier*>(this->name)->toString();
    if (res)
        cout << "Switch to " << name << endl;
    else
        throw EvalException("Fail: " + name + " does not exist");
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
        throw EvalException("Fail: not using any database now");
    return res;
}

any AstCreateTable::eval() {
    bool res = global->mm->evalAst(this);
    string name = dynamic_cast<AstIdentifier*>(this->name)->toString();
    if (res)
        cout << "Create table " << name << endl;
    else
        throw EvalException("Fail: cannot create table " + name + " in current context");
    return res;
}

any AstDropTable::eval() {
    bool res = global->mm->evalAst(this);
    string name = dynamic_cast<AstIdentifier*>(this->name)->toString();
    if (res)
        cout << "Delete table " << name << endl;
    else
        throw EvalException("Fail: cannot delete table " + name + " in current context");
    return res;
}

any AstDesc::eval() {
    bool res = global->mm->evalAst(this);
    string name = dynamic_cast<AstIdentifier*>(this->name)->toString();
    if (!res)
        throw EvalException("Fail: cannot describe table" + name + " in current context");
    return res;
}

any AstCreateIndex::eval() {
    bool res = global->mm->evalAst(this);
    string colName = dynamic_cast<AstIdentifier*>(this->colName)->toString();
    string table = dynamic_cast<AstIdentifier*>(this->table)->toString();
    if (res)
        cout << "Create index on " << colName << " of table " << table << endl;
    else
        throw EvalException("Fail: cannot create index on " + colName + " of table " + table + " in current context");
    return res;
}

any AstDropIndex::eval() {
    bool res = global->mm->evalAst(this);
    string colName = dynamic_cast<AstIdentifier*>(this->colName)->toString();
    string table = dynamic_cast<AstIdentifier*>(this->table)->toString();
    if (res)
        cout << "Drop index on " << colName << " of table " << table << endl;
    else
        throw EvalException("Fail: cannot delete index on " + colName + " of table " + table + " in current context");
    return res;
}