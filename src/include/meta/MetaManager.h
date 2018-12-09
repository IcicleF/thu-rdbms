#ifndef META_MANAGER
#define META_MANAGER

#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string>
#include <iostream>
#include <vector>
#include <fstream>

#include "parser/ast.h"

/*
 * MetaManager 是一个专门用于管理元信息和基础文件交互的类。
 * MetaManager 管理下列操作：
 *     创建、删除、查询和切换数据库
 *     创建、删除、查询表
 */
class MetaManager {
    public:
        MetaManager();
        ~MetaManager();

        bool evalAst(AstBase*);
        bool getTableInfo(const char*, void*);

    private:
        std::string workingDB;
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
};

#endif