#include "meta/MetaManager.h"

using namespace std;

MetaManager::MetaManager() {
    ensureDirectory("database");
}

MetaManager::~MetaManager() { }

bool MetaManager::evalAst(AstBase* ast) {
    int type = ast->type;
    switch (type) {
        case AST_CREATEDB:
            return createDatabase(dynamic_cast<AstCreateDB*>(ast));
        case AST_DROPDB:
            return dropDatabase(dynamic_cast<AstDropDB*>(ast));
        case AST_SHOWDB:
            showDatabases();
            return true;
        case AST_USEDB:
            return useDatabase(dynamic_cast<AstUseDB*>(ast));
        case AST_CREATETABLE:
            return createTable(dynamic_cast<AstCreateTable*>(ast));
        case AST_DROPTABLE:
            return dropTable(dynamic_cast<AstDropTable*>(ast));
        case AST_SHOWTABLES:
            showTables();
            return true;
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
            if (!strcmp(p->d_name, ".") && !strcmp(p->d_name, ".."))
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
    return ensureDirectory(("database/" + dbName).c_str());
}

bool MetaManager::dropDatabase(AstDropDB* ast) {
    std::string dbName = dynamic_cast<AstIdentifier*>(ast->name)->toString();
    if (dbName == workingDB)
        workingDB.clear();
    return removeDirectory(("database/" + dbName).c_str()) == 0;
}

vector<string> MetaManager::showDatabases() {
    vector<string> res;
    DIR* dir = opendir("database");
    int r = 0;
    if (dir) {
        dirent* p;
        while (!r && (p = readdir(dir))) {
            if (!strcmp(p->d_name, ".") && !strcmp(p->d_name, ".."))
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
        closedir(dir);
        return true;
    }
    else
        workingDB.clear();
    return false;
}

bool MetaManager::createTable(AstCreateTable* ast) {
    string dbName = dynamic_cast<AstIdentifier*>(ast->name)->toString();
    string dbDir = "database/" + dbName;
    //...
}

bool MetaManager::dropTable(AstDropTable* ast) {
    string dbName = dynamic_cast<AstIdentifier*>(ast->name)->toString();
}

bool MetaManager::showTables(vector<string>& res) {
    if (workingDB.length() == 0) {
        res.clear();
        return false;
    }
    string dbDir = "database/" + workingDB;

    DIR* dir = opendir(dbDir.c_str());
    int r = 0;
    if (dir) {
        dirent* p;
        while (!r && (p = readdir(dir))) {
            if (!strcmp(p->d_name, ".") && !strcmp(p->d_name, ".."))
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
    return true;
}