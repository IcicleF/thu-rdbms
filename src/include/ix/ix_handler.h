#ifndef IX_HANDLER
#define IX_HANDLER

#include "errs.h"
#include "rid.h"
#include "ix/bplus.h"
#include "rm/rm_scanner.h"

class IXHandler {
    public:
        AttrType attrtype;
        BPlusTree *bpt;

        IXHandler(BPlusTree *bpt, AttrType attrtype);
        IXHandler();
        ~IXHandler();
        int insertEntry(void*, const RID&);
        int deleteEntry(void*, const RID&);
};

#endif