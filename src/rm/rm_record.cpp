#include <cstring>
#include "rm_record.h"

using std::string;

RMRecord::RMRecord() {
    // Nothing special.
}

RMRecord::RMRecord(const RID& rid, const char* data) {
    this->rid = rid;
    this->data = string(data);
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
    strcpy(dest, data.c_str());
}

RID RMRecord::getRID() const {
    return rid;
}