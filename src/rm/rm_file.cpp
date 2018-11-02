#include "rm/rm_file.h"
#include "pf/utils/pagedef.h"
#include "errs.h"
#include <cstring>
#include <cassert>

int ind;

RMFile::RMFile() {
    // Nothing special.
}

RMFile::RMFile(BufPageManager* bpmgr, int fileId) {
    this->bpmgr = bpmgr;
    this->fileId = fileId;
    getRecordSize();
}

RMFile::~RMFile() {
    // Nothing special.
}

RMRecord RMFile::getRec(const RID& rid) const {
    if (bpmgr == NULL) {
        raise(E_RM_NOBUFMGR);
        return RMRecord();
    }
    CharBufType b = (CharBufType)bpmgr->getPage(fileId, rid.getPage(), ind);
    int slot = rid.getSlot();
    if (slot < 0 || slot + extRecSize > PAGE_SIZE) {
        raise(E_RM_INVSLOT);
        return RMRecord();
    }
    char* recbuf = new char[recSize + 5];
    for (int i = 0; i < recSize; ++i)
        recbuf[i] = b[slot + i];
    recbuf[recSize] = 0;
    RMRecord rec(rid, recbuf);
    delete[] recbuf;
    return rec;
}

RID RMFile::insertRec(const char* data) {
    if (bpmgr == NULL) {
        raise(E_RM_NOBUFMGR);
        return RID();
    }
    CharBufType b;
    int page = 0;
    while (true) {
        b = (CharBufType)bpmgr->getPage(fileId, page, ind);
        if (b[USED_SIZE_LOC] == 0)
            formatPage(b);
        if (getEmptyPtr(b) != 0)
            break;
        ++page;
    }
    ushort slot = getEmptyPtr(b);
    ushort used = getUsedSize(b);
    if (slot > used) {
        raise(E_RM_INVEPTR);
        return RID();
    }
    else {
        bpmgr->markDirty(ind);
        for (int i = 0; i < recSize; ++i)
            b[slot + i] = data[i];
        ushort* prev = PREV(b + slot);
        ushort* next = NEXT(b + slot);
        assert(*prev == 0);
        if (slot == used) {         // un-formatted empty slot
            used += extRecSize;
            *next = (used + extRecSize > PAGE_SIZE) ? 0 : used;
            setUsedSize(b, used);
        }
        else if (*next != 0) {      // formatted empty slot
            ushort* nprev = PREV(b + *next);
            *nprev = 0;
        }
        setEmptyPtr(b, *next);
        ushort occ = getOccPtr(b);
        ushort* f_nprev = PREV(b + occ);
        *f_nprev = slot;
        *next = occ;
        setOccPtr(b, slot);
    }
    return RID(page, slot);
}

void RMFile::deleteRec(const RID& rid) {
    if (bpmgr == NULL) {
        raise(E_RM_NOBUFMGR);
        return;
    }
    CharBufType b = (CharBufType)bpmgr->getPage(fileId, rid.getPage(), ind);
    int slot = rid.getSlot();
    if (slot < 0 || slot + extRecSize > PAGE_SIZE) {
        raise(E_RM_INVSLOT);
        return;
    }
    ushort* prev = PREV(b + slot);
    ushort* next = NEXT(b + slot);
    if (*prev != 0) {
        ushort* pnext = NEXT(b + *prev);
        *pnext = *next;
    }
    if (*next != 0) {
        ushort* nprev = PREV(b + *next);
        *nprev = *prev;
    }
    *prev = *next = 0;
    return;
}

void RMFile::updateRec(const RMRecord& rec) {
    if (bpmgr == NULL) {
        raise(E_RM_NOBUFMGR);
        return;
    }
    RID rid = rec.getRID();
    CharBufType b = (CharBufType)bpmgr->getPage(fileId, rid.getPage(), ind);
    int slot = rid.getSlot();
    if (slot < 0 || slot + extRecSize > PAGE_SIZE) {
        raise(E_RM_INVSLOT);
        return;
    }
    char* recbuf = new char[recSize + 5];
    rec.getData(recbuf);
    for (int i = 0; i < recSize; ++i)
        b[slot + i] = recbuf[i];
    delete[] recbuf;
    return;
}