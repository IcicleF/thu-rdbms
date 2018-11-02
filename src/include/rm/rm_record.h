#ifndef RM_RECORD_H
#define RM_RECORD_H

#include <string>
#include "rid.h"

/*
 * RMRecord is unchangeable.
 * 
 *   When a record is updated, the old RMRecord instance should
 * be discarded, and the user should load a new RMRecord instance
 * from an RMFile or RMScanner instance.
 */
class RMRecord {
    public:
        RMRecord();
        RMRecord(const RMRecord&);
        RMRecord(const RID&, const char*);
        RMRecord(const RID&, const std::string& data);
        ~RMRecord();

        void getData(char*) const;
        RID getRID() const;
    
    private:
        std::string data;
        RID rid;
};

#endif