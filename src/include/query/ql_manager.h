#ifndef QL_MANAGER
#define QL_MANAGER

#include "parser/ast.h"
#include "meta/info.h"
#include "rm/rm_manager.h"
#include "rm/rm_file.h"
#include "ix/ix_manager.h"
#include "ix/ix_handler.h"
#include "ix/ix_scanner.h"
#include "rid.h"
#include <cmath>

class QLManager{
    public:
        DBInfo *db_info;
        QLManager();
        QLManager(RMManager*, IXManager*);
        
        bool evalAst(AstBase*);

        bool Insert(AstInsert*);
        bool Delete();
        bool Select();
        bool Update();
    private:
        RMManager *rm;
        IXManager *ix;

        bool checktype(AstLiteral*, ColInfo*);
};

#endif