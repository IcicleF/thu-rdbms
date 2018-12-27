#ifndef QL_MANAGER
#define QL_MANAGER

#include "parser/ast.h"
#include "meta/info.h"
#include "rm/rm_manager.h"
#include "ix/ix_manager.h"

class QLManager{
    public:
        DBInfo *db_info;
        QLManager();
        QLManager(RMManager*, IXManager*);
        
        bool evalAst(AstBase*);

        void Insert();
        void Delete();
        void Select();
        void Update();
    private:
        RMManager *rm;
        IXManager *ix;
};

#endif