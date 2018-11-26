#ifndef IX_HANDLER
#define IX_HANDLER

#include "errs.h"
#include "rid.h"
#include "scanner.h"
#include "ix/bplus.h"

class IXHandler {
    public:
        AttrType attrtype;
        BPlusTree *bpt;
        int attrlen;

        IXHandler(BPlusTree *bpt, AttrType attrtype, int attrlen);
        IXHandler();
        ~IXHandler();
        int insertEntry(void*, const RID&);
        int deleteEntry(void*, const RID&);
};

#endif