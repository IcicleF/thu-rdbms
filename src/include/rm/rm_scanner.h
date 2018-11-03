#ifndef RM_SCANNER_H
#define RM_SCANNER_H

#include "rid.h"
#include "rm/rm_record.h"
#include "rm/rm_file.h"
#include "pf/bufmanager/BufPageManager.h"
#include "pf/fileio/FileManager.h"
#include "pf/utils/pagedef.h"
#include<vector>

enum AttrType {
    INTEGER,
    FLOAT,
    STRING
};
enum ScanType {
    ST_EQ,
    ST_LT,
    ST_GT,
    ST_LE,
    ST_GE,
    ST_NE,
    ST_NOP
};

class RMScanner {
    public:
        RMScanner();
        ~RMScanner();

        void openScan(RMFile&, AttrType, int, int, ScanType, void*);
        bool nextRec(RMRecord&);
        void closeScan();
    private:
        RMFile *rmf;
        int Length,Offset,curpageId;//the total number of records & the present tag location
        ushort curslotId;
        CharBufType curb;
        AttrType type;
        ScanType comp;
        void *value;
        bool vcompare(CharBufType);
        void GetNextRec();
};

#endif
