#ifndef META_MANAGER
#define META_MANAGER

#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <fstream>
#include <iterator>

#include "parser/ast.h"
#include "rm/rm_manager.h"
#include "ix/ix_manager.h"
#include "meta/info.h"
#include "meta/ql_manager.h"
#include "scanner.h"

/*
 * MetaManager 是一个专门用于管理元信息和基础文件交互的类。
 * MetaManager 管理下列操作：
 *     创建、删除、查询和切换数据库
 *     创建、删除、查询表
 */
class MetaManager {
    public:
        MetaManager();
        MetaManager(RMManager*, IXManager*, QLManager*);
        ~MetaManager();

        bool evalAst(AstBase*);
        bool getTableInfo(const char*, void*);
        int dbnum;
        std::map <std::string, DBInfo*> dbMap;

    private:
        std::string workingDB;
        RMManager *rm;
        IXManager *ix;
        QLManager *ql;
        
        DBInfo* InitDB(std::string);
        TableInfo* InitTable(std::string);

        bool ensureDirectory(const char*);
        int removeDirectory(const char*);

        // 数据库相关的内部接口
        bool createDatabase(AstCreateDB*);
        bool dropDatabase(AstDropDB*);
        std::vector<std::string> showDatabases();
        bool useDatabase(AstUseDB*);

        // 表相关的内部接口
        bool createTable(AstCreateTable*);
        bool dropTable(AstDropTable*);
        bool showTables(std::vector<std::string>&);
        bool descTable(AstDesc*);
        bool createIndex(AstCreateIndex*);
        bool dropIndex(AstDropIndex*);
};

#endif