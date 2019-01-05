#ifndef QL_MANAGER
#define QL_MANAGER

#include "parser/ast.h"
#include "meta/info.h"
#include "rm/rm_manager.h"
#include "rm/rm_file.h"
#include "rm/rm_record.h"
#include "ix/ix_manager.h"
#include "ix/ix_handler.h"
#include "ix/ix_scanner.h"
#include "query/where_checker.h"
#include "rid.h"
#include <cmath>
#include <cstring>
#include <string>
#include <vector>
#include <iterator>
#include <map>

struct IndexRM{
    RID rid;
    char* index;

    IndexRM(RID rs, char* inx){
        rid = RID(rs.getPage(), rs.getSlot());
        index = new char[4];
        memcpy(index, inx, 4);
    }
};

class QLManager{
    public:
        DBInfo *db_info;
        QLManager();
        QLManager(RMManager*, IXManager*);
        
        bool evalAst(AstBase*);

        bool Insert(AstInsert*);
        bool Delete(AstDelete*);
        bool Select(AstSelect*);
        bool Update(AstUpdate*);
    private:
        RMManager *rm;
        IXManager *ix;
        void DeleteCol(std::string, IndexRM*);
        void UpdateCol(std::string, RMRecord, const std::map<std::string, ExprType*>&);
        bool checktype(AstLiteral*, ColInfo*);
        bool checkforeignkey(std::string, std::string);
};

#endif