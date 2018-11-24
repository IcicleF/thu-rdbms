#ifndef IX_HANDLER
#define IX_HANDLER

#include "errs.h"
#include "rid.h"

class IXHandler {
    public:
        IXHandler();
        ~IXHandler();

        int insertEntry(void*, const RID&);
        int deleteEntry(void*, const RID&);
};

#endif