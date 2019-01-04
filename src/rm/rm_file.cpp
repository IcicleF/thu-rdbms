#include "rm/rm_file.h"
#include "pf/utils/pagedef.h"
#include "errs.h"
#include <cstring>
#include <cassert>
#include <iostream>
using namespace std;

RMFile::RMFile() {
    // Nothing special.
}

RMFile::RMFile(BufPageManager* bpmgr, int fileId) {
    this->bpmgr = bpmgr;
    this->fileId = fileId;
    getRecordSize();
    //cout << "recordSize is " << recSize << endl;
}

RMFile::~RMFile() {
    // Nothing special.
}

RMRecord RMFile::getRec(const RID& rid) const {
    int ind;
    if (bpmgr == NULL) {
        raise(E_RM_NOBUFMGR);
        return RMRecord();
    }
    CharBufType b = (CharBufType)bpmgr->getPage(fileId, rid.getPage(), ind);
    BufType bb = (BufType)b;
    int slot = rid.getSlot();
    if (slot < 0 || slot + extRecSize > PAGE_SIZE) {
        raise(E_RM_INVSLOT);
        return RMRecord();
    }
    string recbuf;
    recbuf.resize(recSize + 1);
    for (int i = 0; i < recSize; ++i)
        recbuf[i] = b[slot + i];
    recbuf[recSize] = 0;
    RMRecord rec(rid, recbuf);
    return rec;
}

RID RMFile::insertRec(const char* data) {
    int ind;
    if (bpmgr == NULL) {
        raise(E_RM_NOBUFMGR);
        return RID();
    }
    CharBufType b;
    int page = 0;
    while (true) {
        if (page > totPages) {
            setTotPages(totPages + 1);
            b = (CharBufType)bpmgr->getPage(fileId, page, ind);
            formatPage(b);
        }
        else
            b = (CharBufType)bpmgr->getPage(fileId, page, ind);
        if (getEmptyPtr(b) != 0)
            break;
        bpmgr->release(ind);
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
        *prev = 0;
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
        if (occ != 0) {
            ushort* f_nprev = PREV(b + occ);
            *f_nprev = slot;
        }
        *next = occ;
        setOccPtr(b, slot);
        bpmgr->writeBack(ind);
    }
    return RID(page, slot);
}

void RMFile::deleteRec(const RID& rid) {
    int ind;
    if (bpmgr == NULL) {
        raise(E_RM_NOBUFMGR);
        return;
    }
    CharBufType b = (CharBufType)bpmgr->getPage(fileId, rid.getPage(), ind);
    bpmgr->markDirty(ind);
    int slot = rid.getSlot();
    if (slot < 0 || slot + extRecSize > PAGE_SIZE) {
        raise(E_RM_INVSLOT);
        return;
    }
    ushort* prev = PREV(b + slot);
    ushort* next = NEXT(b + slot);
    cout << *prev << " " << *next << endl;
    if (*prev != 0) {
        ushort* pnext = NEXT(b + *prev);
        *pnext = *next;
    }
    if (*next != 0) {
        ushort* nprev = PREV(b + *next);
        *nprev = *prev;
    }
    *prev = *next = 0;
    bpmgr->writeBack(ind);
    return;
}

void RMFile::updateRec(const RMRecord& rec) {
    int ind;
    if (bpmgr == NULL) {
        raise(E_RM_NOBUFMGR);
        return;
    }
    RID rid = rec.getRID();
    CharBufType b = (CharBufType)bpmgr->getPage(fileId, rid.getPage(), ind);
    bpmgr->markDirty(ind);
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
    bpmgr->writeBack(ind);
    return;
}