#ifndef RM_RECORD_H
#define RM_RECORD_H

#include "rid.h"

class RMRecord {
    public:
        RMRecord();
        ~RMRecord();

        void getData(char*) const;
        RID getRID() const;
};

#endif