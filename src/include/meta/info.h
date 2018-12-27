#ifndef DB_INFO
#define DB_INFO

#include <cstdio>
#include "scanner.h"
#include <cstring>
#include <string>
#include <map>
#include <vector>

class ColInfo{
    public:
        std::string name;
        AttrType type;
        int AttrLength,AttrOffset,collimit;
        bool isprimary,isforeign,notnull;
        std::string ref,refcol;

        ColInfo(){
            isprimary = false;
            isforeign = false;
            notnull = false;
            collimit = -1;//no limit
        }
};

class TableInfo{
    public:
        std::string name;
        int colnum,recSize;
        std::map <std::string, ColInfo*> ColMap;
        
        std::map <std::string, int> IndexMap;//rank = 1
        int newindexid,newid;

        TableInfo(){
            colnum = 0;
            ColMap.clear();
            IndexMap.clear();
            recSize = 0;
            newindexid = 1;
            newid = 1;
        }
};

class DBInfo{
    public:
        std::string name;
        int tablenum;
        std::map <std::string, TableInfo*> TableMap;

        DBInfo(){
            tablenum = 0;
            TableMap.clear();
        }
};

#endif