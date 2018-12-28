#ifndef IX_SCANNER
#define IX_SCANNER

#include "errs.h"
#include "rid.h"
#include "scanner.h"
#include "ix/bplus.h"
#include "ix/ix_handler.h"

class IXScanner {
    public:
        IXScanner();
        ~IXScanner();
        
        void openScan(IXHandler&, ScanType, void*);
        bool nextRec(RID&, void*);
        void closeScan();
    private:
        void getnextRec();
        void *standard;
        BPlusTree *bpt;
        BPlusNode cur,stdcur;
        bool pf;
        int curchild,stdpos;
        int attrlen;
        int scanstatus;//1 means scanning 0 means nots
        ScanType scantype;
  
};


#endif