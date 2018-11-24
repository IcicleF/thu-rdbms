#ifndef IX_HANDLER
#define IX_HANDLER

#include "errs.h"
#include "rid.h"
#include "ix/bplus.h"

class IXHandler {
    public:
        BPlusTree *bpt;

        IXHandler(BPlusTree *bpt);
        IXHandler();
        ~IXHandler();
        int insertEntry(void*, const RID&);
        int deleteEntry(void*, const RID&);
};

#endif