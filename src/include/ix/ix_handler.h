#ifndef IX_HANDLER
#define IX_HANDLER

#include "errs.h"
#include "rid.h"
#include "scanner.h"
#include "ix/bplus.h"

#include <string>

class IXHandler {
    public:
        AttrType attrtype;
        BPlusTree *bpt;
        int attrlen;
        std::string fileName;

        IXHandler(BPlusTree *bpt, AttrType attrtype, int attrlen);
        IXHandler();
        ~IXHandler();
        int insertEntry(void*, const RID&);
        int deleteEntry(void*, const RID&);
};

#endif