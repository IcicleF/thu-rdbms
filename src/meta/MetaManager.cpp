#include "meta/MetaManager.h"
#include "global.h"

using namespace std;

extern Global* global;

MetaManager::MetaManager() {
    ensureDirectory("database");
}

MetaManager::~MetaManager() { }

bool MetaManager::evalAst(AstBase* ast) {
    int type = ast->type;
    switch (type) {
        case AST_CREATEDB:
            return createDatabase(dynamic_cast<AstCreateDB*>(ast));
        case AST_DROPDB:
            return dropDatabase(dynamic_cast<AstDropDB*>(ast));
        case AST_SHOWDB:
            global->strList = showDatabases();
            return true;
        case AST_USEDB:
            return useDatabase(dynamic_cast<AstUseDB*>(ast));
        case AST_CREATETABLE:
            return createTable(dynamic_cast<AstCreateTable*>(ast));
        case AST_DROPTABLE:
            return dropTable(dynamic_cast<AstDropTable*>(ast));
        case AST_SHOWTABLES:
            return showTables(global->strList);
    }
    return false;
}

bool MetaManager::ensureDirectory(const char* directory) {
    DIR* dir = opendir(directory);
    if (dir) {
        closedir(dir);
        return false;
    }
    else if (errno == ENOENT) {
        mkdir(directory, 0700);
        return true;
    }
    else {
        cout << "Failed creating directory: " << directory << endl;
        exit(0);
    }
}

int MetaManager::removeDirectory(const char* directory) {
    DIR* dir = opendir(directory);
    int r = 0;
    if (dir) {
        dirent* p;
        while (!r && (p = readdir(dir))) {
            if (!strcmp(p->d_name, ".") && !strcmp(p->d_name, ".."))
                continue;
            string subdir = directory;
            subdir += "/";
            subdir += p->d_name;
            struct stat sbuf;
            if (!stat(subdir.c_str(), &sbuf)) {
                if (S_ISDIR(sbuf.st_mode))
                    r = removeDirectory(subdir.c_str());
                else
                    r = unlink(subdir.c_str());
            }
        }
        closedir(dir);
    }
    if (!r)
        rmdir(directory);
    return r;
}

bool MetaManager::createDatabase(AstCreateDB* ast) {
    std::string dbName = dynamic_cast<AstIdentifier*>(ast->name)->toString();
    return ensureDirectory(("database/" + dbName).c_str());
}

bool MetaManager::dropDatabase(AstDropDB* ast) {
    std::string dbName = dynamic_cast<AstIdentifier*>(ast->name)->toString();
    if (dbName == workingDB)
        workingDB.clear();
    return removeDirectory(("database/" + dbName).c_str()) == 0;
}

vector<string> MetaManager::showDatabases() {
    vector<string> res;
    DIR* dir = opendir("database");
    int r = 0;
    if (dir) {
        dirent* p;
        while (!r && (p = readdir(dir))) {
            if (!strcmp(p->d_name, ".") && !strcmp(p->d_name, ".."))
                continue;
            string subdir = "database/";
            subdir += p->d_name;
            struct stat sbuf;
            if (!stat(subdir.c_str(), &sbuf))
                if (S_ISDIR(sbuf.st_mode))
                    res.push_back(string(p->d_name));
        }
        closedir(dir);
    }
    return res;
}

bool MetaManager::useDatabase(AstUseDB* ast) {
    string dbName = dynamic_cast<AstIdentifier*>(ast->name)->toString();
    string dirName = "database/" + dbName;
    DIR* dir = opendir(dirName.c_str());
    if (dir) {
        workingDB = dbName;
        closedir(dir);
        return true;
    }
    else
        workingDB.clear();
    return false;
}

bool MetaManager::createTable(AstCreateTable* ast) {
    if (workingDB.length() == 0)return false;

    string tableName = dynamic_cast<AstIdentifier*>(ast->name)->toString();
    string tableDir = "database/" + workingDB + "/" + tableName;
    string MetaName = tableDir + "/meta.txt";
    bool ret = ensureDirectory(tableDir.c_str());
    if (ret == false) return false;

    ofstream ctable;
    ctable.open(MetaName.c_str());
    
    ctable << tableName << endl;//tablename

    AstType *temptype;
    AstFieldList *tfl = dynamic_cast<AstFieldList*>(ast->fieldList);
    AstIdentList *til;
    string tempident;

    ctable << tfl->fieldList.size() << endl;//fieldnum

    vector<string> collist;
    collist.clear();
    int colnum = 0;
    for (auto f : tfl->fieldList){
        if (f->type == AST_FIELD)colnum++;
    }
    
    ctable << colnum << endl;//all col ident
    for (auto f : tfl->fieldList){
        if (f->type == AST_FIELD){
            tempident = dynamic_cast<AstIdentifier*>(dynamic_cast<AstField*>(f)->name)->toString();
            ctable << tempident << " ";
            collist.push_back(tempident);
        }
    }

    bool pcheck = true;
    bool exist;

    string Ref,RefCol;
    string Temp;
    int refcolnum;

    for (auto f : tfl->fieldList){
        ctable << f->type << " ";//fieldtype
        if(f->type == AST_FIELD){//field
            temptype = dynamic_cast<AstType*>(dynamic_cast<AstField*>(f)->ftype);
            ctable << temptype->val << " ";//type
            if(dynamic_cast<AstField*>(f)->isNotNull == true){
                ctable << "T" << " ";//isNotNull
            }
            else{
                ctable << "F" << " ";
            }
            if(temptype->val == TYPE_INT || temptype->val == TYPE_CHAR || temptype->val == TYPE_VARCHAR){
                ctable << temptype->len << endl;//AttrLen
            }
        }
        else if (f->type == AST_PRIMKEYDECL){
            til = dynamic_cast<AstIdentList*>(dynamic_cast<AstPrimaryKeyDecl*>(f)->colList);
            ctable << til->identList.size() << " ";//primary col num
            for (auto id : til->identList){
                tempident = dynamic_cast<AstIdentifier*>(id)->toString();
                exist = false;
                for (int i = 0; i < collist.size(); i++){
                    if(tempident == collist[i])exist = true;
                }
                if (exist == false)pcheck = false;
                ctable << tempident << " ";
            }
            ctable << endl;
        }
        else if (f->type == AST_FOREKEYDECL){
            tempident = dynamic_cast<AstIdentifier*>(dynamic_cast<AstForeignKeyDecl*>(f)->colName)->toString();
            ctable << tempident << " ";

            Ref = dynamic_cast<AstIdentifier*>(dynamic_cast<AstForeignKeyDecl*>(f)->ref)->toString();
            RefCol = dynamic_cast<AstIdentifier*>(dynamic_cast<AstForeignKeyDecl*>(f)->refColName)->toString();

            string Reftable = "database/" + workingDB + "/" + Ref + "/meta.txt";
            ifstream iref(Reftable.c_str());
            if(!iref)pcheck = false;
            else{
                ctable << Ref << " ";
                ctable << RefCol << " ";
                iref >> Temp;
                iref >> refcolnum >> refcolnum;
                exist = false;
                for (int i = 0; i < refcolnum; i++){
                    iref >> Temp;
                    if(Temp == RefCol)exist = true;
                }
                if(exist == false)pcheck = false;
            }
            ctable << endl;
        }
    }

    if(pcheck == false){
        remove(MetaName.c_str());
        return false;
    }

    return true;
    /*
    database/dbname/tablename/meta.txt
    tableName
    fieldNum
    colNum
    colName*
    fieldtype ...*
    */
    //...
}

bool MetaManager::dropTable(AstDropTable* ast) {
    if (workingDB.length() == 0)return false;

    string tableName = dynamic_cast<AstIdentifier*>(ast->name)->toString();
    string tableDir = "database/" + workingDB + "/" + tableName;
    return removeDirectory(tableDir.c_str()) == 0;
}

bool MetaManager::showTables(vector<string>& res) {
    res.clear();
    if (workingDB.length() == 0)
        return false;
    string dbDir = "database/" + workingDB;

    DIR* dir = opendir(dbDir.c_str());
    if (dir) {
        dirent* p;
        while (p = readdir(dir)) {
            if (!strcmp(p->d_name, ".") && !strcmp(p->d_name, ".."))
                continue;
            string subdir = dbDir + "/" + p->d_name;
            struct stat sbuf;
            if (!stat(subdir.c_str(), &sbuf))
                if (S_ISDIR(sbuf.st_mode))
                    res.push_back(string(p->d_name));
        }
        closedir(dir);
        return true;
    }
    return false;
}

bool MetaManager::descTable(AstDesc *ast){
    if (workingDB.length() == 0) return false;

    string tablename = dynamic_cast<AstIdentifier*>(ast->name)->toString();
    string metaname = "database/" + workingDB + "/" + tablename + "/meta.txt";
    ifstream ift;
    ift.open(metaname.c_str());
    if(!ift)return false;
    
}