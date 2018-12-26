#include "meta/MetaManager.h"
#include "global.h"

using namespace std;

extern Global* global;

MetaManager::MetaManager() {
    dbMap.clear();
    dbnum = 0;
    ensureDirectory("database");
    DIR *dir;
    dir = opendir("database");
    int r = 0;
    if (dir) {
        dirent* p;
        while (!r && (p = readdir(dir))) {
            if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
                continue;
            string subdir = "database/";
            subdir += p->d_name;
            struct stat sbuf;
            if (!stat(subdir.c_str(), &sbuf))
                if (S_ISDIR(sbuf.st_mode)){
                    dbnum++;
                    dbMap[string(p->d_name)] = InitDB(string(p->d_name));
                }
        }
        closedir(dir);
    }
}

MetaManager::MetaManager(RMManager *rm, IXManager *ix, QLManager *ql)
{
    this->rm = rm;
    this->ix = ix;
    this->ql = ql;
    dbMap.clear();
    dbnum = 0;
    ensureDirectory("database");
    DIR *dir;
    dir = opendir("database");
    int r = 0;
    if (dir) {
        dirent* p;
        while (!r && (p = readdir(dir))) {
            if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
                continue;
            string subdir = "database/";
            subdir += p->d_name;
            struct stat sbuf;
            if (!stat(subdir.c_str(), &sbuf))
                if (S_ISDIR(sbuf.st_mode)){
                    dbnum++;
                    dbMap[string(p->d_name)] = InitDB(string(p->d_name));
                }
        }
        closedir(dir);
    }
}

MetaManager::~MetaManager() { }

DBInfo* MetaManager::InitDB(string dbName)
{
    DBInfo *nt;
    nt = new DBInfo();
    nt->name = dbName;
    string dbDir;
    dbDir = "database/" + dbName;

    DIR* dir = opendir(dbDir.c_str());
    if (dir) {
        dirent* p;
        while (p = readdir(dir)) {
            if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
                continue;
            string subdir = dbDir + "/" + p->d_name;
            struct stat sbuf;
            if (!stat(subdir.c_str(), &sbuf))
                if (S_ISDIR(sbuf.st_mode)){
                    nt->tablenum++;
                    nt->TableMap[string(p->d_name)] = InitTable(subdir);
                }
        }
        closedir(dir);
    }
    return nt;
}

TableInfo* MetaManager::InitTable(string tabledir)
{
    string Metadir = tabledir + "/meta.txt";
    int idx,len;
    string tbName;

    len = tabledir.length();
    idx = tabledir.find("/", 0);
    tbName = tabledir.substr(idx+1, len-idx);

    TableInfo* nt;
    nt = new TableInfo();
    nt->name = tbName;

    ifstream ift;
    ift.open(Metadir.c_str());
    if (!ift) return nt;

    int fieldnum,colnum,recSize;
    string field,colname,tablename,notnull;
    int fieldtype,coltype,colspace;

    ColInfo* tcol;
    ift >> tablename;
    nt->name = tablename;
    ift >> fieldnum >> colnum;
    nt->colnum = colnum;

    recSize = 0;
    for (int i = 0; i < colnum; i++){
        ift >> colname >> coltype >> colspace;
        tcol = new ColInfo();
        nt->ColMap[colname] = tcol;

        tcol->name = colname;
        tcol->AttrLength = colspace;
        tcol->AttrOffset = recSize;
        tcol->isforeign = false;
        tcol->isprimary = false;
        tcol->notnull = false;
        tcol->collimit = -1;
        recSize += colspace;
        if(coltype == TYPE_INT){
            tcol->type = INTEGER;
        }
        else if(coltype == TYPE_FLOAT){
            tcol->type = FLOAT;
        }
        else if(coltype == TYPE_CHAR || coltype == TYPE_DATE){
            tcol->type = STRING;
        }
    }

    nt->recSize = recSize;

    int colindex,collimit,prinum;
    string priname,ref,refcol;

    ColInfo *tempcol;

    for (int i = 0; i < fieldnum; i++){
        ift >> fieldtype;
        if(fieldtype == AST_FIELD){
            ift >> colname >> coltype >> notnull;
            tempcol = nt->ColMap[colname];
            if (notnull[0] == 'T'){
                tempcol->notnull = true;
            }
            else tempcol->notnull = false;
            
            if(coltype == TYPE_INT || coltype == TYPE_CHAR || coltype == TYPE_FLOAT){
                ift >> collimit;
                tempcol->collimit = collimit;
            }
        }
        else if (fieldtype == AST_PRIMKEYDECL){
            ift >> prinum;
            for (int j = 0; j < prinum; j++){
                ift >> priname;
                tempcol = nt->ColMap[priname];
                tempcol->isprimary = true;
            }
        }
        else if (fieldtype == AST_FOREKEYDECL){
            ift >> priname >> ref >> refcol;
            tempcol = nt->ColMap[priname];
            tempcol->isforeign = true;
            tempcol->ref = ref;
            tempcol->refcol = refcol;
        }
    }

    ift.close();
    return nt;

}

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
        case AST_DESC:
            return descTable(dynamic_cast<AstDesc*>(ast));
        case AST_CREATEINDEX:
            return createIndex(dynamic_cast<AstCreateIndex*>(ast));
        case AST_DROPINDEX:
            return dropIndex(dynamic_cast<AstDropIndex*>(ast));
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
            if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
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
    dbnum++;
    DBInfo* nt;
    nt = new DBInfo();
    nt->name = dbName;
    dbMap[dbName] = nt;
    return ensureDirectory(("database/" + dbName).c_str());
}

bool MetaManager::dropDatabase(AstDropDB* ast) {
    std::string dbName = dynamic_cast<AstIdentifier*>(ast->name)->toString();
    if(dbMap[dbName] != NULL){
        delete dbMap[dbName];
        dbMap[dbName] = NULL;
        dbnum--;
    }
    if (dbName == workingDB){
        ql->db_info = NULL;
        workingDB.clear();
    }
    return removeDirectory(("database/" + dbName).c_str()) == 0;
}

vector<string> MetaManager::showDatabases() {
    vector<string> res;
    DIR* dir = opendir("database");
    int r = 0;
    if (dir) {
        dirent* p;
        while (!r && (p = readdir(dir))) {
            if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
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
        ql->db_info = dbMap[dbName];
        closedir(dir);
        return true;
    }
    else{
        workingDB.clear();
        ql->db_info = NULL;
    }
    return false;
}

bool MetaManager::createTable(AstCreateTable* ast) {
    if (workingDB.length() == 0)return false;

    string tableName = dynamic_cast<AstIdentifier*>(ast->name)->toString();
    string tableDir = "database/" + workingDB + "/" + tableName;
    string MetaName = tableDir + "/meta.txt";
    string DateName = tableDir + "/data.txt";
    bool ret = ensureDirectory(tableDir.c_str());
    if (ret == false) return false;

    ofstream ctable;
    ifstream iref;

    ctable.open(MetaName.c_str());
    
    ctable << tableName << endl;//tablename

    AstType *temptype;
    AstFieldList *tfl = dynamic_cast<AstFieldList*>(ast->fieldList);
    AstIdentList *til;
    string tempident;

    ctable << tfl->fieldList.size() << endl;//fieldnum

    vector<string> collist;
    collist.clear();
    int recSize = 0;
    int colnum = 0;

    for (auto f : tfl->fieldList){
        if (f->type == AST_FIELD)colnum++;
    }
    
    ctable << colnum << endl;//colnum
    
    int limitlen; //the size for char
    int TempcolType,TempcolSpace;
    string Temp;
    string Ref,RefCol;
    int refcolnum;
    bool pcheck = true;
    bool exist;

    for (auto f : tfl->fieldList){
        //cout << "#" << f->type << endl;
        if (f->type == AST_FIELD){
            tempident = dynamic_cast<AstIdentifier*>(dynamic_cast<AstField*>(f)->name)->toString();
            ctable << tempident << " ";
            temptype = dynamic_cast<AstType*>(dynamic_cast<AstField*>(f)->ftype);
            ctable << temptype->val << " ";//type
            if (temptype->val == TYPE_INT || temptype->val == TYPE_FLOAT){
                recSize += 4;
                ctable << 4;
            }
            else if (temptype->val == TYPE_CHAR) {
                limitlen = dynamic_cast<AstLiteral*>(temptype->len)->val;//AttrLen
                recSize += limitlen;
                ctable << limitlen;
            }
            else if (temptype->val == TYPE_DATE) {
                recSize += 10;
                ctable << 10;
            }
            collist.push_back(tempident);
            ctable << endl;
        }
    }
    

    for (auto f : tfl->fieldList){
        ctable << f->type << " ";//fieldtype
        if(f->type == AST_FIELD){//field
            tempident = dynamic_cast<AstIdentifier*>(dynamic_cast<AstField*>(f)->name)->toString();
            ctable << tempident << " ";

            temptype = dynamic_cast<AstType*>(dynamic_cast<AstField*>(f)->ftype);
            ctable << temptype->val << " ";//type
            if(dynamic_cast<AstField*>(f)->isNotNull == true){
                ctable << "T" << " ";//isNotNull
            }
            else{
                ctable << "F" << " ";
            }
            if(temptype->val == TYPE_INT || temptype->val == TYPE_CHAR || temptype->val == TYPE_VARCHAR){
                ctable << dynamic_cast<AstLiteral*>(temptype->len)->val << endl;//AttrLen
            }
        }
        else if (f->type == AST_PRIMKEYDECL){
            til = dynamic_cast<AstIdentList*>(dynamic_cast<AstPrimaryKeyDecl*>(f)->colList);
            ctable << til->identList.size() << " ";//primary col num
            for (auto id : til->identList){
                tempident = dynamic_cast<AstIdentifier*>(id)->toString();
                exist = false;
                //cout << "! " << tempident << endl;
                for (int i = 0; i < collist.size(); i++){
                    //cout << collist[i] << endl;
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
            
            exist = false;
            //cout << "! " << tempident << endl;
            for (int i = 0; i < collist.size(); i++){
                //cout << collist[i] << endl;
                if(tempident == collist[i])exist = true;
            }

            Ref = dynamic_cast<AstIdentifier*>(dynamic_cast<AstForeignKeyDecl*>(f)->ref)->toString();
            RefCol = dynamic_cast<AstIdentifier*>(dynamic_cast<AstForeignKeyDecl*>(f)->refColName)->toString();

            string Reftable = "database/" + workingDB + "/" + Ref + "/meta.txt";
            iref.open(Reftable.c_str());
            if(!iref)pcheck = false;
            else{
                ctable << Ref << " ";
                ctable << RefCol << " ";
                iref >> Temp;
                iref >> refcolnum >> refcolnum;
                exist = false;
                for (int i = 0; i < refcolnum; i++){
                    iref >> Temp >> TempcolType >> TempcolSpace;
                    if(Temp == RefCol)exist = true;
                }
                if(exist == false)pcheck = false;
                iref.close();
            }
            ctable << endl;
        }
    }
    ctable.close();
    if(pcheck == false){
        remove(MetaName.c_str());
        return false;
    }
    else{
        DBInfo *tempdb = dbMap[workingDB];
        TableInfo* nt = InitTable(tableDir);
        tempdb->tablenum++;
        tempdb->TableMap[tableName] = nt;
    }

    return true;
    /*
    database/dbname/tablename/meta.txt
    tableName
    fieldNum
    colNum
    (colName, colType, colSpace)*
    fieldtype ...*
    */
    //...
}

bool MetaManager::dropTable(AstDropTable* ast) {
    if (workingDB.length() == 0)return false;

    string tableName = dynamic_cast<AstIdentifier*>(ast->name)->toString();
    string tableDir = "database/" + workingDB + "/" + tableName;
    
    DBInfo* tempdb = dbMap[workingDB];
    if (tempdb->TableMap[tableName] != NULL){
        delete tempdb->TableMap[tableName];
        tempdb->TableMap[tableName] = NULL;
        tempdb->tablenum--;
    }

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
            if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
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
    string temp;
    int tempcoltype,tempcolspace;

    ift >> temp;
    cout << "table name: " << temp << endl;
    int fieldnum,colnum;
    ift >> fieldnum;

    ift >> colnum;
    for (int i = 0; i < colnum; i++)ift>>temp>>tempcoltype>>tempcolspace;

    int fieldtype,coltype,collen,prikeylen;

    for(int i = 0; i < fieldnum; i++){
        ift >> fieldtype;
        if (fieldtype == AST_FIELD){
            ift >> temp;
            cout << temp << " ";//colname
            ift >> coltype;
            ift >> temp;//not null
            if(coltype == TYPE_INT){
                ift >> collen;
                cout << "INT(" << collen << ") ";
            }
            if(coltype == TYPE_FLOAT){
                cout << "FLOAT ";
            }
            if(coltype == TYPE_DATE){
                cout << "DATE ";
            }
            if(coltype == TYPE_CHAR){
                ift >> collen;
                cout << "CHAR(" << collen <<") ";
            }
            if(coltype == TYPE_VARCHAR){
                ift >> collen;
                cout << "VARCHAR(" << collen << ") ";
            }
            if (temp[0] == 'T')cout << "NOT NULL";
            cout << endl;
        }
        if (fieldtype == AST_PRIMKEYDECL){
            cout << "PRIMARY KEY ( ";
            ift >> prikeylen;
            for (int i = 0; i < prikeylen; i++){
                ift >> temp;
                cout << temp << " ";
            }
            cout << ")" << endl;
        }
        if (fieldtype == AST_FOREKEYDECL){
            cout << "FOREIGN KEY ";
            ift >> temp;
            cout << temp;
            cout << " REFERENCES ";
            ift >> temp;
            cout << temp << " ";
            ift >> temp;
            cout << " ( " << temp << " )" << endl;
        }
    }
    ift.close();
    return true;
}

bool MetaManager::createIndex(AstCreateIndex* ast)
{
    string tablename,colname;
    if (workingDB.length() == 0) return false;
    tablename = dynamic_cast<AstIdentifier*>(ast->table)->toString();
    colname = dynamic_cast<AstIdentifier*>(ast->colName)->toString();
    string tabledir,indexdir,metadir;
    tabledir = "database/" + workingDB + "/" + tablename;
    indexdir = tabledir + "/index.txt";
    metadir = tabledir + "/meta.txt";

    ifstream ift;
    ofstream oft;
    
    int indexnum;
    vector<string> indices;
    indices.clear();
    string temp;

    ift.open(indexdir.c_str());
    if (!ift)indexnum = 0;
    else{
        ift >> indexnum;
        for (int i = 0; i < indexnum; i++){
            ift >> temp;
            indices.push_back(temp);
        }
    }
    ift.close();

    bool exist = false;
    ift.open(metadir.c_str());
    if(!ift)return false;
    ift >> temp;
    int colnum;
    ift >> colnum >> colnum;
    for (int i = 0 ; i < colnum; i++){
        ift >> temp;
        if (colname == temp)exist = true;
    }
    ift.close();
    if (exist == false) return false;
    
    indices.push_back(colname);
    indexnum++;
    oft.open(indexdir.c_str());
    oft << indexnum << endl;
    for(int i = 0; i < indexnum; i++){
        oft << indices[i] << endl;
    }
    oft.close();
    return true;
}

bool MetaManager::dropIndex(AstDropIndex* ast){
    string tablename,colname;
    if (workingDB.length() == 0) return false;
    tablename = dynamic_cast<AstIdentifier*>(ast->table)->toString();
    colname = dynamic_cast<AstIdentifier*>(ast->colName)->toString();
    string tabledir,indexdir,metadir;
    tabledir = "database/" + workingDB + "/" + tablename;
    indexdir = tabledir + "/index.txt";
    metadir = tabledir + "/meta.txt";

    ifstream ift;
    ofstream oft;
    
    int indexnum;
    vector<string> indices;
    indices.clear();
    string temp;
    bool exist = false;

    ift.open(indexdir.c_str());
    if (!ift)indexnum = 0;
    else{
        ift >> indexnum;
        for (int i = 0; i < indexnum; i++){
            ift >> temp;
            if (temp == colname){
                exist = true;
                indexnum--;
            }
            else indices.push_back(temp);
        }
    }
    ift.close();
    if(exist == false)return false;

    oft.open(indexdir.c_str());
    oft << indexnum << endl;
    for(int i = 0; i < indexnum; i++){
        oft << indices[i] << endl;
    }
    oft.close();
    return true; 
}