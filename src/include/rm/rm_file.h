#ifndef RM_FILE_H
#define RM_FILE_H

#include "rid.h"
#include "rm/rm_record.h"

class RMFile {
    public:
        RMFile();
        ~RMFile();

        RMRecord getRec(const RID&) const;
        RID insertRec(const char*);
        void deleteRec(const RID&);
        void updateRec(const RMRecord&);
        void forcePages() const;
};

#endif