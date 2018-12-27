#include "query/ql_manager.h"

QLManager::QLManager()
{
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
            return Delete();
        case AST_UPDATE:
            return Update();
        case AST_SELECT:
            return Select();
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
    int limval,year,month,day;

    if (cl->collimit != -1){
        if (temptype == INTEGER){
            if (cl->collimit < 10){
                limval = 10^(cl->collimit);
                if (ast->val >= limval) return false;
            }
        }
        else if(temptype == STRING){
            strval = string(ast->strval);
            if(strval.length() > cl->collimit) return false;
        }
    }

    if (cl->asttype == TYPE_DATE){
        strval = string(ast->strval);
        if (strval.length() != 10) return false;
        for (int i = 0; i < 10; i++){
            if (i == 4 || i == 7) {
                if (strval[i] != '-' && strval[i] != '/') return false;
            }
            else{
                if (strval[i] > '9' || strval[i] < '0') return false;
            }
        }
        year = atoi(strval.substr(0,4).c_str());
        month = atoi(strval.substr(5,2).c_str());
        day = atoi(strval.substr(8,2).c_str());
        if (year < 2013 || year > 2019) return false;
        if (month < 1 || month > 12) return false;
        int limit[13];
        limit[1] = 31; limit[3] = 31; limit[5] = 31;
        limit[7] = 31; limit[8] = 31; limit[10] = 31; limit[12] = 31;
        limit[4] = 30; limit[6] = 30; limit[9] = 30; limit[11] = 30;
        if (year % 4 == 0)limit[2] = 29;
        else limit[2] = 28;
        if (day < 1 || day > limit[month]) return false;
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

    for(int i = 0; i < sz; i++){
        vil = dynamic_cast<AstValList*>(vallists->valLists[i]);
        int vsz = vil->valList.size();
        if(vsz != tb->colnum)pcheck = false;
        if(!pcheck)break;
        for(int j = 0; j < vsz; j++){
            cl = tb->ColMap[tb->cols[j]];
            if (vil->valList[j]->type == AST_KEYWORD){
                kwd = dynamic_cast<AstKeyword*>(vil->valList[j]);
                if (kwd->name != "SQLNULL") pcheck = false;
                if (cl->notnull == true) pcheck = false;
            }
            else if (vil->valList[j]->type == AST_LITERAL){
                pcheck = checktype(lil, cl);
                if(cl->isprimary){
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
                    if (isc.nextRec(temprid)) pcheck = false;
                    isc.closeScan();
                    ix->closeIndex(*ih);
                }
                if(cl->isforeign){
                    refTable = db_info->TableMap[cl->ref];
                    refCol = refTable->ColMap[cl->refcol];
                    indexdir = "database/" + db_info->name + "/" + refTable->name + "/index";
                    ih = ix->openIndex(indexdir.c_str(), refTable->IndexMap[refCol->name]);
                    if (refCol->type == INTEGER)isc.openScan((*ih), ST_EQ, (void*)(&lil->val));
                    if (refCol->type == FLOAT)isc.openScan((*ih), ST_EQ, (void*)(&lil->floatval));
                    if (refCol->type == STRING)isc.openScan((*ih), ST_EQ, (void*)(lil->strval));
                    if(isc.nextRec(temprid)) pcheck = false;
                    isc.closeScan();
                    ix->closeIndex(*ih);
                }
            }
            else pcheck = false;
            if(!pcheck)break;
        }
    }
    if(!pcheck)return false;
    
    string datadir = "database/" + db_info->name + "/" + tableName + "/data.txt";
    RMFile rh;
    rh = rm->openFile(datadir.c_str());
    char* Data;
    
    indexdir = "database/" + db_info->name + "/" + tableName + "/index";

    for (int i = 0; i < sz; i++){
        vil = dynamic_cast<AstValList*>(vallists->valLists[i]);
        int vsz = vil->valList.size();
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
        rm->closeFile(rh);
        ih = ix->openIndex(indexdir.c_str(), 0);
        ih->insertEntry((void *)(&(tb->newid)), temprid);
        ix->closeIndex(*ih);

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
                ix->closeIndex(*ih);
            }
        }
    }
    return true;
}

bool QLManager::Delete()
{
    return false;
}

bool QLManager::Update()
{
    return false;
}

bool QLManager::Select()
{
    return false;
}