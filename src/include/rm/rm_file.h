#ifndef RM_FILE_H
#define RM_FILE_H

#include <string>
#include "rid.h"
#include "rm/rm_record.h"
#include "pf/bufmanager/BufPageManager.h"

class RMFile {
    public:
        int fileId;

    public:
        RMFile();
        RMFile(BufPageManager*, int);
        ~RMFile();

        RMRecord getRec(const RID&) const;
        RID insertRec(const char*);
        void deleteRec(const RID&);
        void updateRec(const RMRecord&);
    
    private:
        int recSize;
        BufPageManager* bpmgr;
        
        void getRecordSize() {
            BufType b = bpmgr->getPage(fileId, 0, index);
            recSize = int(b[0]) + sizeof(short);
        }
        static void formatPage(CharBufType cb) {
            ShortBufType b = (ShortBufType)cb;
            b[EMPTY_PTR_LOC] = PAGE_HEADER;
            b[OCC_PTR_LOC] = 0;
            b[USED_SIZE_LOC] = PAGE_HEADER;
        }
        static ushort getEmptyPtr(CharBufType cb) {
            ShortBufType b = (ShortBufType)cb;
            return b[EMPTY_PTR_LOC];
        }
        static void setEmptyPtr(CharBufType cb, ushort ptr) {
            ShortBufType b = (ShortBufType)cb;
            b[EMPTY_PTR_LOC] = ptr;
        }
        static ushort getOccPtr(CharBufType cb) {
            ShortBufType b = (ShortBufType)cb;
            return b[OCC_PTR_LOC];
        }
        static void setOccPtr(CharBufType cb, ushort ptr) {
            ShortBufType b = (ShortBufType)cb;
            b[OCC_PTR_LOC] = ptr;
        }
        static ushort getUsedSize(CharBufType cb) {
            ShortBufType b = (ShortBufType)cb;
            return b[USED_SIZE_LOC];
        }
        static void setUsedSize(CharBufType cb, ushort size) {
            ShortBufType b = (ShortBufType)cb;
            b[USED_SIZE_LOC] = size;
        }
};

#endif