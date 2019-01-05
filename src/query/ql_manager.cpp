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
            return Update(dynamic_cast<AstUpdate*>(ast));
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
    if(temptype != cl->type)
        if (!(temptype == INTEGER && cl->type == FLOAT))
            return false;
    
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
    AstLiteral *colval;
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
    IXHandler *ih0, *ih1;
    IXScanner isc;
    RID temprid;
    char* tempinx;

    //printf("sz = %d\n", sz);

    for(int i = 0; i < sz; i++){
        //printf("   i = %d\n", i);
        vil = dynamic_cast<AstValList*>(vallists->valLists[i]);
        int vsz = vil->valList.size();
        if(vsz != tb->colnum){
            pcheck = false;
            break;
        }
        //printf("   vsz = %d\n", vsz);
        for(int j = 0; j < vsz; j++){
            //printf("      j = %d\n", j);
            cl = tb->ColMap[tb->cols[j]];
            if (vil->valList[j]->type == AST_KEYWORD){
                if (cl->notnull == true || cl->isprimary == true) pcheck = false;
            }
            else if (vil->valList[j]->type == AST_LITERAL){
                colval = dynamic_cast<AstLiteral*>(vil->valList[j]);
                pcheck = checktype(colval, cl);
                if (!pcheck)
                    break;
                if(cl->isprimary){//repeat primary key check
                    indexdir = "database/" + db_info->name + "/" + tableName + "/index";
                    ih1 = ix->openIndex(indexdir.c_str(), tb->IndexMap[cl->name]);
                    tempinx = new char[ih1->attrlen];
                    //printf("      index opened\n");
                    if(cl->type == INTEGER){
                        if(intmap[colval->val] != 0)pcheck = false;
                        intmap[colval->val] = 1;
                        isc.openScan((*ih1), ST_EQ, (void*)(&colval->val));
                    }
                    if(cl->type == FLOAT){
                        float v = colval->floatval;
                        if (colval->literalType == L_INT)
                            v = (float)colval->val;
                        if(floatmap[v] != 0)pcheck = false;
                        floatmap[v] = 1;
                        isc.openScan((*ih1), ST_EQ, (void*)(&v));
                    }
                    if(cl->type == STRING){
                        if(stringmap[string(colval->strval)] != 0)pcheck = false;
                        stringmap[string(colval->strval)] = 1;
                        isc.openScan((*ih1), ST_EQ, (void*)(colval->strval));
                    }
                    //printf("      scan opened\n");
                    if (isc.nextRec(temprid, tempinx)) pcheck = false;
                    delete[] tempinx;
                    isc.closeScan();
                    //printf("      scan closed, pcheck=%s\n", pcheck ? "true" : "false");
                    ix->closeIndex(*ih1);
                    //printf("      index closed\n");
                }
                if(cl->isforeign){//existed foreign key check
                    refTable = db_info->TableMap[cl->ref];
                    refCol = refTable->ColMap[cl->refcol];
                    indexdir = "database/" + db_info->name + "/" + refTable->name + "/index";
                    ih1 = ix->openIndex(indexdir.c_str(), refTable->IndexMap[refCol->name]);
                    tempinx = new char[ih1->attrlen];
                    if (refCol->type == INTEGER)isc.openScan((*ih1), ST_EQ, (void*)(&colval->val));
                    if (refCol->type == FLOAT) {
                        float v = colval->floatval;
                        if (colval->literalType == L_INT)
                            v = (float)colval->val;
                        isc.openScan((*ih1), ST_EQ, (void*)(&v));
                    }
                    if (refCol->type == STRING)isc.openScan((*ih1), ST_EQ, (void*)(colval->strval));
                    if(isc.nextRec(temprid, tempinx) == false) pcheck = false;
                    isc.closeScan();
                    ix->closeIndex(*ih1);
                }
            }
            else pcheck = false;
            if(!pcheck)break;
        }
        if (!pcheck)
            break;
    }
    if(!pcheck){
        cout << "illegal input record" << endl;
        return false;
    }
    printf("check fin!\n");
    string datadir = "database/" + db_info->name + "/" + tableName + "/data.txt";
    char* Data;
    
    indexdir = "database/" + db_info->name + "/" + tableName + "/index";

    int nullint = -1;
    float nullfloat = -1;
    char* nullstr = new char[8192];
    memset(nullstr, 0, sizeof(nullstr));
    nullstr[0] = (char)(-1);

    for (int i = 0; i < sz; i++){
        //cout << "i = " << i << endl;
        vil = dynamic_cast<AstValList*>(vallists->valLists[i]);
        Data = new char[tb->recSize];
        memset(Data, 0, tb->recSize);
        for (int j = 0; j < tb->colnum; j++){
            cl = tb->ColMap[tb->cols[j]];
            //printf("   offset %d\n", cl->AttrOffset);
            if (vil->valList[j]->type == AST_KEYWORD){
                if (cl->type == INTEGER){
                    memcpy(Data + cl->AttrOffset, (void *)(&nullint), cl->AttrLength);
                }
                if (cl->type == FLOAT){
                    memcpy(Data + cl->AttrOffset, (void *)(&nullfloat), cl->AttrLength);
                }
                if (cl->type == STRING){
                    memcpy(Data + cl->AttrOffset, nullstr, cl->AttrLength);
                }
            }
            else {
                colval = dynamic_cast<AstLiteral*>(vil->valList[j]);
                if (cl->type == INTEGER){
                    memcpy(Data + cl->AttrOffset, (void *)(&colval->val), cl->AttrLength);
                }
                if (cl->type == FLOAT){
                    float v = colval->floatval;
                    if (colval->literalType == L_INT)
                        v = (float)colval->val;
                    memcpy(Data + cl->AttrOffset, (void *)(&v), cl->AttrLength);
                }
                if (cl->type == STRING){
                    strcpy(Data + cl->AttrOffset, colval->strval);
                }
            }
        }

        //printf("data is (%d)<", tb->recSize);
        //for (int i = 0; i < tb->recSize; ++i)
        //    printf("%02x ", (unsigned char)Data[i]);
        //printf(">\n");

        RMFile rh = rm->openFile(datadir.c_str());
        temprid = rh.insertRec(Data);
        rm->closeFile(rh);

        //cout << "insert at pg " << temprid.getPage() << " sl " << temprid.getSlot() << endl;

        ih0 = ix->openIndex(indexdir.c_str(), 0);
        ih1 = ix->openIndex(indexdir.c_str(), 1);
        
        ih0->insertEntry((void *)(&(tb->newid)), temprid);
        tb->updateNewid();

        //printf("   index0 updated!\n");

        for (int j = 0; j < tb->colnum; j++){
            cl = tb->ColMap[tb->cols[j]];
            if (cl->isprimary){
                //printf("   %d col update primary\n", j);
                try {
                    colval = dynamic_cast<AstLiteral*>(vil->valList[j]);
                }
                catch (exception e) {
                    cout << "primary key cannot be null" << endl;
                    return false;
                }
                if (cl->type == INTEGER){
                    ih1->insertEntry((void*)(&colval->val), temprid);
                }
                if (cl->type == FLOAT){
                    float v = colval->floatval;
                    if (colval->literalType == L_INT)
                        v = (float)colval->val;
                    ih1->insertEntry((void*)(&v), temprid);
                }
                if (cl->type == STRING){
                    ih1->insertEntry((void*)colval->strval, temprid);
                }
                //printf("   index1 updated!\n");
            }
        }
        delete[] Data;

        if (i % 1000 == 999) {
            printf("Processed %d records", i + 1);
            if (i + 1 != sz)
                printf(", please be patient...\n");
            else
                printf(".\n");
            //ih0->bpt->printTree();
        }
        
        ix->closeIndex(*ih0);
        ix->closeIndex(*ih1);
    }

    delete[] nullstr;
    return true;
}

bool QLManager::checkvalue(string tableName, string colName, ExprType chkval)
{
    string datadir = "database/" + db_info->name + "/" + tableName + "/data.txt";
    string indexdir = "database/" + db_info->name + "/" + tableName + "/index";

    IXHandler* ih = ix->openIndex(indexdir.c_str(), 0);
    IXScanner isc;
    char* tempinx;

    TableInfo* tb;
    ColInfo* cl;
    tb = db_info->TableMap[tableName];
    cl = tb->ColMap[colName];
    tempinx = new char[ih->attrlen];
    isc.openScan(*ih, ST_NOP, tempinx);

    RMFile rh = rm->openFile(datadir.c_str());

    RID temprid;
    RMRecord rmc;
    ExprType scanval;
    
    bool exist = false;

    while(isc.nextRec(temprid, tempinx)){
        rmc = rh.getRec(temprid);
        scanval = getColumn(rmc, tableName, colName);
        if (cl->type == INTEGER){
            if (scanval.val == chkval.val) exist = true;
        }
        else if (cl->type == FLOAT){
            if (fabs(scanval.floatval - chkval.floatval) < 1e-7) exist = true;
        }
        else if (cl->type == STRING){
            if (strncmp(scanval.strval, chkval.strval, cl->AttrLength) == 0) exist = true;
        }
        if (exist) break;
    }

    isc.closeScan();
    ix->closeIndex(*ih);
    rm->closeFile(rh);
    delete[] tempinx;
    
    return exist;

}

bool QLManager::checkforeignkey(string tableName, string colName, ExprType chkval)
{
    map<string, TableInfo*>::iterator tb_it;
    map<string, ColInfo*>::iterator cl_it;

    TableInfo* tb;
    ColInfo* cl;
    tb = db_info->TableMap[tableName];
    cl = tb->ColMap[colName];
    
    string tbName;
    TableInfo* refTable;
    bool has_foreign = false;
    string clName;
    ColInfo* refCol;

    for (tb_it = db_info->TableMap.begin(); tb_it != db_info->TableMap.end(); tb_it++){
        tbName = tb_it->first;
        refTable = tb_it->second;
        for (cl_it = refTable->ColMap.begin(); cl_it != refTable->ColMap.end(); cl_it++){
            refCol = cl_it->second;
            if (refCol->isforeign && refCol->ref == tableName && refCol->refcol == colName){
                has_foreign = checkvalue(refTable->name, refCol->name, chkval);
                if (has_foreign == true)return false;
            }
        }
    }
    return true;

}

void QLManager::DeleteCol(string tableName, IndexRM* rx)
{
    TableInfo* tb = db_info->TableMap[tableName];
    string inxdir = "database/" + db_info->name + "/" + tableName + "/index";
    string datadir = "database/" + db_info->name + "/" + tableName + "/data.txt";
    IXHandler* ih = ix->openIndex(inxdir.c_str(), 0);

    ih->deleteEntry((void*)rx->index, rx->rid);
    ix->closeIndex(*ih);
    cout << "Delete Entry0 finished" << endl;
    RMFile rh;
    RMRecord rmc;            


    rh = rm->openFile(datadir.c_str());
    
    rmc = rh.getRec(rx->rid);    
    char* tempinx;
    tempinx = new char[tb->recSize];            
    rmc.getData(tempinx);

    rh.deleteRec(rx->rid);    
    rm->closeFile(rh);
    cout << "delete record finished" << endl;

    for (int i = 0 ; i < tb->cols.size(); i++){
        if (tb->IndexMap[tb->cols[i]] != 0){
            ih = ix->openIndex(inxdir.c_str(), tb->IndexMap[tb->cols[i]]);
            
            char* inx_for_del;
            
            ColInfo* cl = tb->ColMap[tb->cols[i]];
            inx_for_del = new char[cl->AttrLength];
            memcpy(inx_for_del, tempinx + cl->AttrOffset, cl->AttrLength);

            ih->deleteEntry(inx_for_del, rx->rid);
            ix->closeIndex(*ih);
            delete[] inx_for_del;

        }
    }
    delete[] tempinx;
}

bool QLManager::Delete(AstDelete* ast)
{
    if (db_info == NULL) return false;


    vector<IndexRM*> dels;
    dels.clear();

    string tableName = dynamic_cast<AstIdentifier*>(ast->table)->toString();
    string datadir = "database/" + db_info->name + "/" + tableName + "/data.txt";
    string inxdir = "database/" + db_info->name + "/" + tableName + "/index";

    ColInfo* cl;
    TableInfo* tb;
    map<string, ColInfo*>::iterator itv;
    tb = db_info->TableMap[tableName];
    if (tb == NULL) return false;
    string priname;
    for (itv = tb->ColMap.begin(); itv != tb->ColMap.end(); itv++){
        cl = itv->second;
        if (cl->isprimary) priname = cl->name;
    }


    IXHandler* ih = ix->openIndex(inxdir.c_str(), 0);
    IXScanner isc;
    RMFile rh = rm->openFile(datadir.c_str());
    RMRecord rmc;
    ExprType chk;
    char* tempinx;
    RID temprid;

    tempinx = new char[ih->attrlen];
    map <string, RMRecord> recmap;
    recmap.clear();

    isc.openScan(*ih, ST_NOP, tempinx);
    IndexRM *tempr;

    bool has_foreign = true;

    while (isc.nextRec(temprid, tempinx)) {
        recmap[tableName] = rh.getRec(temprid);
        rmc = rh.getRec(temprid);
        if (checkWhere(ast->whereClause, recmap)){
            tempr = new IndexRM(temprid, tempinx);
            dels.push_back(tempr);
            chk = getColumn(rmc, tableName, priname);
            if (checkforeignkey(tableName, priname, chk) == false) has_foreign = false;
        }
    }
    isc.closeScan();
    ix->closeIndex(*ih);    
    rm->closeFile(rh);

    if (has_foreign == false){
        delete[] tempinx;
        return false;
    }

    for (int i = 0; i < dels.size(); i++){        
        cout << "delete index " << i << endl;
        cout << *((int *)dels[i]->index) << ":" << dels[i]->rid.getPage() << ":" << dels[i]->rid.getSlot() << endl;
        DeleteCol(tableName, dels[i]);
    }
    cout << "delsize " << dels.size() << endl;
    delete[] tempinx;
    return true;    

}

void QLManager::UpdateCol(string tableName, RMRecord rec, const map<string, ExprType*>& recmap)
{
    TableInfo* tb = db_info->TableMap[tableName];
    ExprType* ts;
    map<string, ExprType*>::const_iterator itv;
    ColInfo* cl;
    string colName;
    string indexdir = "database/" + db_info->name + "/" + tableName + "/index";
    string datadir = "database/" + db_info->name + "/" + tableName + "/data.txt";

    char* upd_data;
    upd_data = new char[tb->recSize];
    RID updrid;
    rec.getData(upd_data);
   /* cout << "data before update " << endl;
    for (int i = 0; i < tb->recSize; i++){
        printf("%02x ", upd_data[i]);
    }
    cout << endl;*/
    updrid = rec.getRID();

    IXHandler *ih;
    for (itv = recmap.begin(); itv != recmap.end(); itv++){
        ts = itv->second;
        colName = itv->first;
        //cout << "upd_colName:" << colName << endl;
        //cout << ts->val << endl;

        cl = tb->ColMap[colName];
        char* inx_for_del;            
        inx_for_del = new char[cl->AttrLength];
//delete old index
        if (tb->IndexMap[colName] != 0){
            ih = ix->openIndex(indexdir.c_str(), tb->IndexMap[colName]);
            //cout << tb->IndexMap[colName] << endl;
            memcpy(inx_for_del, upd_data + cl->AttrOffset, cl->AttrLength);
            ih->deleteEntry(inx_for_del, updrid);
        }
        if (cl->type == INTEGER)memcpy(upd_data + cl->AttrOffset, (char*)(&(ts->val)), cl->AttrLength);
        if (cl->type == FLOAT)memcpy(upd_data + cl->AttrOffset, (char*)(&(ts->floatval)), cl->AttrLength);
        if (cl->type == STRING)strncpy(upd_data + cl->AttrOffset, ts->strval, cl->AttrLength);
//insert new index
        if (tb->IndexMap[colName] != 0){
            memcpy(inx_for_del, upd_data + cl->AttrOffset, cl->AttrLength);
            ih->insertEntry(inx_for_del, updrid);
            ix->closeIndex(*ih);
        }
        delete[] inx_for_del;
    }
    /*
    cout << "data after update " << endl;
    for (int i = 0; i < tb->recSize; i++){
        printf("%02x ", upd_data[i]);
    }
    cout << endl;
    cout << updrid.getPage() << " " << updrid.getSlot() << endl;*/
    string s_for_upd;
    s_for_upd.resize(tb->recSize + 1);
    for (int i = 0; i < tb->recSize; i++)s_for_upd[i] = upd_data[i];
    RMRecord newrec(updrid, s_for_upd);
    RMFile rh = rm->openFile(datadir.c_str());
    rh.updateRec(newrec);
    rm->closeFile(rh);
    delete[] upd_data;
}

bool QLManager::Update(AstUpdate* ast)
{
    if (db_info == NULL) return false;
    string tableName = dynamic_cast<AstIdentifier*>(ast->table)->toString();
    TableInfo* tb = db_info->TableMap[tableName];
    if (tb == NULL) return false;

    string datadir = "database/" + db_info->name + "/" + tableName + "/data.txt";
    string inxdir = "database/" + db_info->name + "/" + tableName + "/index";

    string colname;
    AstSetClause* setclause = dynamic_cast<AstSetClause*>(ast->setClause);
    AstSet* st;
    AstLiteral* colval;
    ColInfo* cl;

    ExprType* upd_val;
    map <string, ExprType*> updcolmap;
    updcolmap.clear();
    bool update_primary = false;
    string priname;

    TableInfo* refTable;
    ColInfo* refCol;
    string indexdir;
    char* tempinx;
    IXHandler* ih;
    IXScanner isc;
    bool check_foreign = true;
    RID temprid;

    for (int i = 0 ; i < setclause->setList.size(); i++){
        st = dynamic_cast<AstSet*>(setclause->setList[i]);
        colname = dynamic_cast<AstIdentifier*>(st->colName)->toString();

        cl = tb->ColMap[colname];
        
        if (tb->ColMap[colname] == NULL) return false;
        upd_val = new ExprType();
        upd_val->strval = new char[cl->AttrLength];
        if (st->sval->type == AST_KEYWORD){
            if (cl->notnull == true || cl->isprimary == true) return false;
            for (int j = 0; j < cl->AttrLength; j++)upd_val->strval[i] = 0;
            if (cl->type == INTEGER) {
                upd_val->val = -1;
                upd_val->type = TYPE_INT;
            }
            if (cl->type == FLOAT) {
                upd_val->floatval = -1;
                upd_val->type = TYPE_FLOAT;
            }
            if (cl->type == STRING) {
                upd_val->strval[0] = -1;
                upd_val->type = TYPE_CHAR;
            }
        }
        else {
            if (cl->isprimary == true){
                update_primary = true;
                priname = cl->name;
            }
            colval = dynamic_cast<AstLiteral*>(st->sval);
            if (checktype(colval, cl) == false) return false;
            if (cl->type == INTEGER){
                upd_val->val = colval->val;
                upd_val->type = TYPE_INT;
            }
            if (cl->type == FLOAT){
                if (colval->literalType == L_INT) upd_val->floatval = colval->val;
                else upd_val->floatval = colval->floatval;
                upd_val->type = TYPE_FLOAT;
            }
            if (cl->type == STRING){
                strncpy(upd_val->strval, colval->strval, cl->AttrLength);
                upd_val->type = TYPE_CHAR;
            }            
            if (cl->isforeign == true){
                refTable = db_info->TableMap[cl->ref];
                refCol = refTable->ColMap[cl->refcol];
                indexdir = "database/" + db_info->name + "/" + refTable->name + "/index";
                ih = ix->openIndex(indexdir.c_str(), refTable->IndexMap[refCol->name]);
                tempinx = new char[ih->attrlen];
                if (refCol->type == INTEGER)isc.openScan((*ih), ST_EQ, (void*)(&colval->val));
                if (refCol->type == FLOAT) {
                    float v = colval->floatval;
                    if (colval->literalType == L_INT)
                        v = (float)colval->val;
                    isc.openScan((*ih), ST_EQ, (void*)(&v));
                }
                if (refCol->type == STRING)isc.openScan((*ih), ST_EQ, (void*)(colval->strval));
                if(isc.nextRec(temprid, tempinx) == false) check_foreign = false;
                isc.closeScan();
                ix->closeIndex(*ih);
                delete[] tempinx;
            }
        }
        if (updcolmap[colname] != NULL) return false;
        updcolmap[colname] = upd_val;
    }

    cout << "check for update fin!" << endl;
    if (check_foreign == false) return false;

    ih = ix->openIndex(inxdir.c_str(), 0);
    RMFile rh = rm->openFile(datadir.c_str());

    tempinx = new char[ih->attrlen];
    map <string, RMRecord> recmap;
    recmap.clear();

    vector<RMRecord> upd_rec;
    upd_rec.clear();

    isc.openScan(*ih, ST_NOP, tempinx);
    while (isc.nextRec(temprid, tempinx)) {
        recmap[tableName] = rh.getRec(temprid);
        if (checkWhere(ast->whereClause, recmap)){
            upd_rec.push_back(rh.getRec(temprid));
        }
    }
    isc.closeScan();
    ix->closeIndex(*ih);    
    rm->closeFile(rh);

    cout << "upd size:" << upd_rec.size() << endl;
    delete[] tempinx;

    bool pcheck = true;
    if (update_primary == true){
//check for update primary(the updated primary key already exists)
        if(upd_rec.size() > 1)return false;
        ih = ix->openIndex(inxdir.c_str(), 1);
        tempinx = new char[ih->attrlen];
        upd_val = updcolmap[priname];
        RID upd_rid = upd_rec[0].getRID();
        if (upd_val->type == TYPE_INT)isc.openScan(*ih, ST_EQ, (void*)(&(upd_val->val)));
        if (upd_val->type == TYPE_FLOAT)isc.openScan(*ih, ST_EQ, (void*)(&(upd_val->floatval)));
        if (upd_val->type == TYPE_CHAR)isc.openScan(*ih, ST_EQ, (void*)(upd_val->strval));
        if (isc.nextRec(temprid, tempinx)){
            if (upd_rid.getPage() != temprid.getPage() || upd_rid.getSlot() != temprid.getSlot())
                pcheck = false;
        }
        isc.closeScan();
        ix->closeIndex(*ih);    
        delete[] tempinx;
        rh = rm->openFile(datadir.c_str());
        RMRecord rmc = rh.getRec(temprid);
        ExprType chk = getColumn(rmc, tableName, priname);
        if (checkforeignkey(tableName, priname, chk) == false) pcheck = false;
        if (!pcheck) return false;
    }

    for (int i = 0; i < upd_rec.size(); i++){
        cout << "update " << i << endl;
        UpdateCol(tableName, upd_rec[i], updcolmap);
    }
    return true;
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
            for (auto colInfo : db_info->TableMap[owner]->cols)
                printColumns.push_back(make_pair(owner, db_info->TableMap[owner]->ColMap[colInfo]));
        }
    }
    
    vector<int> printOffset;
    int nCols = printColumns.size();
    printOffset.resize(nCols + 1);
    printOffset[0] = 0;
    // Calculate printing-width for each column
    for (int i = 0; i < nCols; ++i) {
        int titleLen = printColumns[i].first.length() + printColumns[i].second->name.length() + 1;
        int attrWidth = printColumns[i].second->type == FLOAT ? 11 : printColumns[i].second->collimit;
        int width = max(titleLen, attrWidth);
        width = max(width, 4);
        printOffset[i + 1] = printOffset[i] + width + 2;
        cout << printColumns[i].first << "." << printColumns[i].second->name;
        for (int j = printOffset[i] + titleLen; j < printOffset[i + 1]; ++j)
            cout << " ";
    }
    cout << endl;       // Title printed
    for (int i = 0, fi = printOffset[nCols]; i < fi; ++i)
        cout << "-";
    cout << endl;       // Split line printed

    TableEnumerator te(db_info, rm, ix, tableList);
    auto callback = [&](const map<string, RMRecord>& recMp) -> void {
        for (int i = 0, fi = printColumns.size(); i < fi; ++i) {
            auto pc = printColumns[i];
            ExprType res = getColumn(recMp.at(pc.first), pc.first, pc.second->name);
            stringstream ss;
            if (res.type == TYPE_INT) {
                if (res.val != -1)
                    ss << res.val;
                else
                    ss << "null";
            }
            else if (res.type == TYPE_FLOAT) {
                if (res.floatval != -1)
                    ss << res.floatval;
                else
                    ss << "null";
            }
            else {
                if (res.strval[0] != (char)(-1))
                    ss << res.strval;
                else
                    ss << "null";
            }
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