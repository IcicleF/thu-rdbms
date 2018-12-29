#include "query/ql_manager.h"
#include "query/where_checker.h"
#include <climits>
#include <iostream>
#include <sstream>
#include <functional>

using namespace std;

inline int pow10lim(int x) {
    if (x > 10)
        return -1;
    if (x == 10)
        return INT_MAX;
    int res = 1;
    while (x--)
        res *= 10;
    return res - 1;
}

QLManager::QLManager()
{
    db_info = NULL;
    //
}

QLManager::QLManager(RMManager *rm, IXManager *ix)
{
    this->rm = rm;
    this->ix = ix;
    this->db_info = NULL;
}

bool QLManager::evalAst(AstBase* ast)
{
    int type = ast->type;
    switch (type) {
        case AST_INSERT:
            return Insert(dynamic_cast<AstInsert*>(ast));
        case AST_DELETE:
            return Delete(dynamic_cast<AstDelete*>(ast));
        case AST_UPDATE:
            return Update();
        case AST_SELECT:
            return Select(dynamic_cast<AstSelect*>(ast));
    }
    return false;
}

bool QLManager::checktype(AstLiteral* ast, ColInfo* cl)
{
    AttrType temptype;
    if(ast->literalType == L_INT)temptype = INTEGER;
    if(ast->literalType == L_DECIMAL)temptype = FLOAT;
    if(ast->literalType == L_STRING)temptype = STRING;
    if(temptype != cl->type)return false;
    
    string strval;
    int limval;

    if (cl->collimit != -1){
        if (temptype == INTEGER){
            if (cl->collimit < 10){
                limval = pow10lim(cl->collimit);
                if (ast->val > limval) return false;
            }
        }
        else if(temptype == STRING){
            strval = string(ast->strval);
            if(strval.length() > cl->collimit) return false;
        }
    }

    if (cl->asttype == TYPE_DATE){
        strval = string(ast->strval);
        if (!checkDateStr(strval))
            return false;
    }
    return true;
}

bool QLManager::Insert(AstInsert* ast)
{
    if(db_info == NULL)return false;

    string tableName = dynamic_cast<AstIdentifier*>(ast->table)->toString();
    if(db_info->TableMap[tableName] == NULL)return false;
    
    TableInfo* tb = db_info->TableMap[tableName];
    string colname;
    ColInfo* cl;
    bool pcheck = true;

    AstValLists* vallists;
    AstValList* vil;
    AstLiteral *lil;
    AstKeyword *kwd;

    vallists = dynamic_cast<AstValLists*>(ast->valList);

    map<int,int> intmap;
    map<float,int> floatmap;
    map<string,int> stringmap;
    intmap.clear();
    floatmap.clear();
    stringmap.clear();

    int sz = vallists->valLists.size();
    AttrType temptype;

    TableInfo* refTable;
    ColInfo* refCol;

    string indexdir;
    IXHandler* ih;
    IXScanner isc;
    RID temprid;
    char* tempinx;
    tempinx = new char[tb->recSize];

    for(int i = 0; i < sz; i++){
        vil = dynamic_cast<AstValList*>(vallists->valLists[i]);
        int vsz = vil->valList.size();
        if(vsz != tb->colnum){
            pcheck = false;
            break;
        }
        for(int j = 0; j < vsz; j++){
            cl = tb->ColMap[tb->cols[j]];
            if (vil->valList[j]->type == AST_KEYWORD){
                //kwd = dynamic_cast<AstKeyword*>(vil->valList[j]);
                //if (kwd->name != "SQLNULL") pcheck = false;
                if (cl->notnull == true) pcheck = false;
            }
            else if (vil->valList[j]->type == AST_LITERAL){
                pcheck = checktype(lil, cl);
                if (!pcheck)
                    break;
                if(cl->isprimary){//repeat primary key check
                    indexdir = "database/" + db_info->name + "/" + tableName + "/index";
                    ih = ix->openIndex(indexdir.c_str(), tb->IndexMap[cl->name]);
                    if(cl->type == INTEGER){
                        if(intmap[lil->val] != 0)pcheck = false;
                        intmap[lil->val] = 1;
                        isc.openScan((*ih), ST_EQ, (void*)(&lil->val));
                    }
                    if(cl->type == FLOAT){
                        if(floatmap[lil->floatval] != 0)pcheck = false;
                        floatmap[lil->floatval] = 1;
                        isc.openScan((*ih), ST_EQ, (void*)(&lil->floatval));
                    }
                    if(cl->type == STRING){
                        if(stringmap[string(lil->strval)] != 0)pcheck = false;
                        stringmap[string(lil->strval)] = 1;
                        isc.openScan((*ih), ST_EQ, (void*)(lil->strval));
                    }
                    if (isc.nextRec(temprid, tempinx)) pcheck = false;
                    isc.closeScan();
                    ix->closeIndex(*ih);
                }
                if(cl->isforeign){//existed foreign key check
                    refTable = db_info->TableMap[cl->ref];
                    refCol = refTable->ColMap[cl->refcol];
                    indexdir = "database/" + db_info->name + "/" + refTable->name + "/index";
                    ih = ix->openIndex(indexdir.c_str(), refTable->IndexMap[refCol->name]);
                    if (refCol->type == INTEGER)isc.openScan((*ih), ST_EQ, (void*)(&lil->val));
                    if (refCol->type == FLOAT)isc.openScan((*ih), ST_EQ, (void*)(&lil->floatval));
                    if (refCol->type == STRING)isc.openScan((*ih), ST_EQ, (void*)(lil->strval));
                    if(isc.nextRec(temprid, tempinx)) pcheck = false;
                    isc.closeScan();
                    ix->closeIndex(*ih);
                }
            }
            else pcheck = false;
            if(!pcheck)break;
        }
        if (!pcheck)
            break;
    }
    if(!pcheck)return false;
    
    string datadir = "database/" + db_info->name + "/" + tableName + "/data.txt";
    RMFile rh = rm->openFile(datadir.c_str());
    char* Data;
    
    indexdir = "database/" + db_info->name + "/" + tableName + "/index";

    for (int i = 0; i < sz; i++){
        vil = dynamic_cast<AstValList*>(vallists->valLists[i]);
        Data = new char[tb->recSize];
        for (int j = 0; j < tb->colnum; j++){
            cl = tb->ColMap[tb->cols[j]];
            if (cl->type == INTEGER){
                strncpy(Data + cl->AttrOffset, (char *)(&lil->val), cl->AttrLength);
            }
            if (cl->type == FLOAT){
                strncpy(Data + cl->AttrOffset, (char *)(&vil->floatval), cl->AttrLength);
            }
            if (cl->type == STRING){
                strncpy(Data + cl->AttrOffset, vil->strval, cl->AttrLength);
            }
        }
        temprid = rh.insertRec(Data);
        
        ih = ix->openIndex(indexdir.c_str(), 0);
        ih->insertEntry((void *)(&(tb->newid)), temprid);
        tb->newid++;
        rh.setNewid((unsigned int)(tb->newid));

        for (int j = 0; j < tb->colnum; j++){
            cl = tb->ColMap[tb->cols[j]];
            if (cl->isprimary){
                ih = ix->openIndex(indexdir.c_str(), 1);
                if (cl->type == INTEGER){
                    ih->insertEntry((void*)(&lil->val), temprid);
                }
                if (cl->type == FLOAT){
                    ih->insertEntry((void*)(&lil->floatval), temprid);
                }
                if (cl->type == STRING){
                    ih->insertEntry((void*)lil->strval, temprid);
                }
            }
        }
        delete[] Data;
    }
    ih = ix->openIndex(indexdir.c_str(), 0);
    ix->closeIndex(*ih);
    ih = ix->openIndex(indexdir.c_str(), 1);
    ix->closeIndex(*ih);

    rm->closeFile(rh);
    return true;
}

void QLManager::DeleteCol(string tableName, IndexRM rx)
{
    TableInfo* tb = db_info->TableMap[tableName];
    string inxdir = "database/" + db_info->name + "/" + tableName + "/index";
    string datadir = "database/" + db_info->name + "/" + tableName + "/data.txt";
    IXHandler* ih = ix->openIndex(inxdir.c_str(), 0);
    ih->deleteEntry(rx.index, rx.rid);
    ix->closeIndex(*ih);
    RMFile rh;
    RMRecord rmc;
    rh = rm->openFile(datadir.c_str());
    rh.deleteRec(rx.rid);
    char* tempinx;
    tempinx = new char[tb->recSize];
    for (int i = 0 ; i < tb->cols.size(); i++){
        if (tb->IndexMap[tb->cols[i]] != 0){
            ih = ix->openIndex(inxdir.c_str(), tb->IndexMap[tb->cols[i]]);
            
            rmc = rh.getRec(rx.rid);
            char* inx_for_del;
            
            ColInfo* cl = tb->ColMap[tb->cols[i]];
            inx_for_del = new char[cl->AttrLength];
            rmc.getData(tempinx);
            memcpy(inx_for_del, tempinx + cl->AttrOffset, cl->AttrLength);

            ih->deleteEntry(inx_for_del, rx.rid);
            ix->closeIndex(*ih);
        }
    }
}

bool QLManager::Delete(AstDelete* ast)
{
    if (db_info == NULL) return false;

    vector<IndexRM> dels;
    dels.clear();

    string tableName = dynamic_cast<AstIdentifier*>(ast->table)->toString();
    string datadir = "database/" + db_info->name + "/" + tableName + "/data.txt";
    string inxdir = "database/" + db_info->name + "/" + tableName + "/index";

    IXHandler* ih = ix->openIndex(inxdir.c_str(), 0);
    IXScanner isc;
    char* tempinx;
    RID temprid;

    tempinx = new char[ih->attrlen];
    if (ast->whereClause == NULL) {
        isc.openScan(*ih, ST_NOP, tempinx);
        while (isc.nextRec(temprid, tempinx)) {
            dels.push_back(IndexRM(temprid, tempinx));
        }
        isc.closeScan();
    }
    else {

    }
    ix->closeIndex(*ih);
    for (int i = 0; i < dels.size(); i++)DeleteCol(tableName, dels[i]);
    return true;
}

bool QLManager::Update()
{
    return false;
}

typedef function<void(const map<string, RMRecord>&)> CallbackType;
struct TableEnumerator {
    DBInfo* dbInfo;
    RMManager* rm;
    IXManager* ix;

    AstIdentList* tableList;
    vector<RMRecord> selRecs;
    int n;
    char buf[8192];
    
    TableEnumerator(DBInfo* dbInfo, RMManager* rm, IXManager* ix, AstIdentList* tableList = NULL) : 
        dbInfo(dbInfo), rm(rm), ix(ix), tableList(tableList) {
        n = tableList->identList.size();
        selRecs.resize(n);
    }

    void Enumerate(int i, AstBase* where, CallbackType cb) {
        if (i == n) {
            map<string, RMRecord> recMp;
            for (int j = 0; j < n; j++)
                recMp[dynamic_cast<AstIdentifier*>(tableList->identList[j])->toString()] = selRecs[j];
            if (checkWhere(where, recMp))
                cb(recMp);
            return;
        }
        auto tableName = dynamic_cast<AstIdentifier*>(tableList->identList[i])->toString();
        string dataFile = "database/" + dbInfo->name + "/" + tableName + "/data.txt";
        string indexFile = "database/" + dbInfo->name + "/" + tableName + "/index";

        RMFile rf = rm->openFile(dataFile.c_str());
        IXHandler* ih = ix->openIndex(indexFile.c_str(), 0);
        IXScanner isc;
        isc.openScan(*ih, ST_NOP, NULL);
        RID rid;
        while (isc.nextRec(rid, buf)) {
            RMRecord rec = rf.getRec(rid);
            selRecs[i] = rec;
            this->Enumerate(i + 1, where, cb);
        }
        isc.closeScan();
        ix->closeIndex(*ih);
        rm->closeFile(rf);
    }
};

// SELECT [Selector] FROM [TableList] WHERE [WhereClause]
bool QLManager::Select(AstSelect* ast)
{
    if (db_info == NULL)
        return false;
    auto selector = dynamic_cast<AstSelector*>(ast->selector)->colList;
    auto tableList = dynamic_cast<AstIdentList*>(ast->tableList);

    // Check table names in [TableList]
    for (auto tableName : tableList->identList) {
        auto name = dynamic_cast<AstIdentifier*>(tableName)->toString();
        if (db_info->TableMap.find(name) == db_info->TableMap.end())
            throw EvalException("unknown table: " + name);
    }

    vector<pair<string, ColInfo*>> printColumns;
    if (selector) {
        auto colList = dynamic_cast<AstColList*>(selector);
        for (auto colBase : colList->colList) {
            auto col = dynamic_cast<AstCol*>(colBase);
            auto colName = dynamic_cast<AstIdentifier*>(col->colName)->toString();
            string owner;
            if (col->owner) {
                owner = dynamic_cast<AstIdentifier*>(col->owner)->toString();
                bool found = false;

                // Check table names in [Selector]
                for (auto tableName : tableList->identList)
                    if (dynamic_cast<AstIdentifier*>(tableName)->toString() == owner) {
                        found = true;
                        break;
                    }
                if (!found)
                    throw EvalException("unknown table: " + owner);
            }
            else {
                if (tableList->identList.size() > 1)
                    throw EvalException("must specify table name for column " + colName);
                owner = dynamic_cast<AstIdentifier*>(tableList->identList[0])->toString();
            }

            // Use try-catch to check column names in [Selector]
            try {
                auto colInfo = db_info->TableMap[owner]->ColMap.at(colName);
                printColumns.push_back(make_pair(owner, colInfo));
            }
            catch (exception ex) {
                throw EvalException("unknown column: " + colName);
            }
        }
    }
    else {
        for (auto tableName : tableList->identList) {
            auto owner = dynamic_cast<AstIdentifier*>(tableName)->toString();
            for (auto colInfo : db_info->TableMap[owner]->ColMap)
                printColumns.push_back(colInfo);
        }
    }
    
    vector<int> printOffset;
    printOffset.resize(printColumns.size());
    printOffset[0] = 0;
    // Calculate printing-width for each column
    for (int i = 0; i < printColumns.size() - 1; ++i) {
        int titleLen = printColumns[i].first.length() + printColumns[i].second->name.length() + 1;
        int attrWidth = printColumns[i].second->type == FLOAT ? 11 : printColumns[i].second->collimit;
        int width = max(titleLen, attrWidth);
        printOffset[i + 1] = printOffset[i] + width + 2;
        cout << printColumns[i].first << "." << printColumns[i].second->name;
        for (int j = printOffset[i] + printColumns[i].first.length(); j < printOffset[i + 1]; ++j)
            cout << " ";
    }
    cout << endl;       // Title printed
    for (int i = 0, fi = printOffset[printColumns.size() - 1]; i < fi; ++i)
        cout << "-";
    cout << endl;       // Split line printed

    TableEnumerator te(db_info, rm, ix, tableList);
    auto callback = [&](const map<string, RMRecord>& recMp) -> void {
        for (int i = 0, fi = printColumns.size(); i < fi; ++i) {
            auto pc = printColumns[i];
            ExprType res = getColumn(recMp.at(pc.first), pc.first, pc.second->name);
            stringstream ss;
            if (res.type == TYPE_INT)
                ss << res.val;
            else if (res.type == TYPE_FLOAT)
                ss << res.floatval;
            else
                ss << res.strval;
            string s = ss.str();
            cout << s;
            if (i != fi - 1) {
                for (int j = printOffset[i] + s.length(); j < printOffset[i + 1]; ++j)
                    cout << " ";
            }
        }
        cout << endl;
    };
    te.Enumerate(0, ast->whereClause, callback);
    return true;
}