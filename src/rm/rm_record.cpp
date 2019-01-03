#include <cstring>
#include "rm/rm_record.h"

using std::string;

RMRecord::RMRecord() {
    // Nothing special.
}

RMRecord::RMRecord(const RMRecord& b) {
    rid = b.rid;
    data = b.data;
}

RMRecord::RMRecord(const RID& rid, const std::string& data) {
    this->rid = rid;
    this->data = data;
}

RMRecord::~RMRecord() {
    // Nothing special.
}

void RMRecord::getData(char* dest) const {
    // assume that there are enough space in dest.
    memcpy(dest, data.c_str(), data.size() + 1);
}

RID RMRecord::getRID() const {
    return rid;
}