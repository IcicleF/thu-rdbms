#ifndef RM_FILE_H
#define RM_FILE_H

#include <string>
#include "rid.h"
#include "rm/rm_record.h"
#include "pf/bufmanager/BufPageManager.h"

#define PAGE_HEADER 16
#define EMPTY_PTR_LOC 2
#define OCC_PTR_LOC 3
#define USED_SIZE_LOC 4

#define extRecSize (recSize + sizeof(ushort) * 2)
#define PREV(x) ((ushort*)(x + recSize))
#define NEXT(x) (PREV(x) + 1)

class RMFile {
    public:
        int fileId;
        int recSize;
        BufPageManager* bpmgr;

    public:
        RMFile();
        RMFile(BufPageManager*, int);
        ~RMFile();

        RMRecord getRec(const RID&) const;
        RID insertRec(const char*);
        void deleteRec(const RID&);
        void updateRec(const RMRecord&);

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
        void getRecordSize() {
            int ind;
            BufType b = bpmgr->getPage(fileId, 0, ind);
            recSize = int(b[0]);
            std::cout << "recSize is " << recSize << std::endl;
        }
        static void formatPage(CharBufType cb) {
            ShortBufType b = (ShortBufType)cb;
            b[EMPTY_PTR_LOC] = PAGE_HEADER;
            b[OCC_PTR_LOC] = 0;
            b[USED_SIZE_LOC] = PAGE_HEADER;
        }
};

#endif