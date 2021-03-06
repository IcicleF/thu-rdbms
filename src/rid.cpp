#include "rid.h"

RID::RID() {
    page = slot = -1;
}

RID::RID(int page, short slot) {
    this->page = page;
    this->slot = slot;
}

RID::RID(const RID& src) {
    page = src.page;
    slot = src.slot;
}

RID::~RID() {
    // Nothing special.
}

int RID::getPage() const {
    return this->page;
}

short RID::getSlot() const {
    return this->slot;
}