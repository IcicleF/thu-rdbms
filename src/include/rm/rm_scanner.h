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

        void openScan(const RMFile&, AttrType, int, int, ScanType, void*);
        bool nextRec(RMRecord&);
        void closeScan();
    private:
        int limit,bz;//the total number of records & the present tag location
        vector<RMRecord> res;
        void ScanPage(const RMFile&, CharBufType, AttrType, int, int, ScanType, void*);
        bool vcompare(AttrType, ScanType, CharBufType, void*);
};

#endif
