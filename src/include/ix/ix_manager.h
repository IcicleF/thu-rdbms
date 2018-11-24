#ifndef IX_MANAGER_H
#define IX_MANAGER_H

#include "errs.h"
#include "rm/rm_scanner.h"
#include "ix/ix_handler.h"

class IXManager {
    public:
        IXManager();
        ~IXManager();

        int createIndex(const char*, int, AttrType, int);
        int destroyIndex(const char*, int);
        IXHandler openIndex(const char*, int);
        int closeIndex(IXHandler&);
};

#endif // IX_MANAGER_H
