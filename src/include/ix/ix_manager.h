#ifndef IX_MANAGER_H
#define IX_MANAGER_H

#include "ix/ix_handler.h"
#include "pf/bufmanager/BufPageManager.h"
#include "pf/fileio/FileManager.h"
#include "pf/utils/pagedef.h"

#include <string>
#include <map>

class IXManager {
    FileManager *fm;
    BufPageManager *bpm;

    public:
        IXManager();
        IXManager(FileManager *fm, BufPageManager *bpm);
        ~IXManager();

        int createIndex(const char*, int, AttrType, int);
        int destroyIndex(const char*, int);
        IXHandler *openIndex(const char*, int);
        int closeIndex(IXHandler&);

    private:
        std::map<std::string, int> fileIds;
};

#endif // IX_MANAGER_H
