#ifndef BPLUS_H
#define BPLUS_H

#include <cstdlib>
#include <string>
#include <cstring>
#include <functional>
#include "pf/bufmanager/BufPageManager.h"
#include "rid.h"

const short BT_INNER = 1, BT_LEAF = 2;

struct BPlusNode {
    BPlusTree* owner;
    int pageId, pageIndex;
    union {
        BufType page;
        CharBufType ipage;
        ShortBufType spage;
    };

    BPlusNode(int pageId = -1) : pageId(pageId) { }

    // load page from current pageId
    void getPage() {
        page = owner->bpm->getPage(owner->fileId, pageId, pageIndex);
    }

    // access attrLen from owner
    int getAttrLen() const { return owner->attrLen; }

    // get record count in this node
    short count() const { return spage[0]; }
    void setCount(short _count) {
        spage[0] = _count;
        owner->bpm->markDirty(pageIndex);
    }
    
    // get node type
    short type() const { return spage[1]; }
    void setType(short _type) {
        spage[1] = _type;
        owner->bpm->markDirty(pageIndex);
    }

    // get parent node
    int parent() const { return ipage[1]; }
    void setParent(int _p) {
        ipage[1] = _p;
        owner->bpm->markDirty(pageIndex);
    }

    // get parent ptr index to self
    short parentptr() const { return spage[4]; }
    void setParentPtr(short _pp) {
        spage[4] = _pp;
        owner->bpm->markDirty(pageIndex);
    }

    // get address of attribute of record #i
    void* val(int i) const {
        int offs = 16 + (getAttrLen() + 6) * i;
        return (void*)(page + offs);
    }
    void setVal(int i, void* pData) {
        int offs = 16 + (getAttrLen() + 6) * i;
        memcpy(page + offs, pData, getAttrLen());
        owner->bpm->markDirty(pageIndex);
    }

    // get child node #i (from #0)
    int child(int i) const {
        if (i == 0)
            return ipage[2];
        else {
            int offs = 10 + (getAttrLen() + 6) * i;
            return *((int*)(page + offs));
        }
    }
    void setChild(int i, int ch) {
        if (i == 0)
            ipage[2] = ch;
        else {
            int offs = 10 + (getAttrLen() + 6) * i;
            *((int*)(page + offs)) = ch;
        }
        owner->bpm->markDirty(pageIndex);
    }

    // return RID stored in rec #i
    RID rec(int i) const {
        int offs = 10 + (getAttrLen() + 6) * i;
        int* ptrPage = (int*)(page + offs);
        short* ptrSlot = (short*)(page + offs + 4);
        return RID(*ptrPage, *ptrSlot);
    }
    void setRec(int i, const RID& rid) {
        int offs = 10 + (getAttrLen() + 6) * i;
        int* ptrPage = (int*)(page + offs);
        short* ptrSlot = (short*)(page + offs + 4);
        *ptrPage = rid.getPage();
        *ptrSlot = rid.getSlot();
        owner->bpm->markDirty(pageIndex);
    }
};

class BPlusTree {
    friend struct BPlusNode;

    public:
        BPlusTree();
        BPlusTree(BufPageManager*, int);
        ~BPlusTree();

        bool searchEntry(void*, RID&);
        bool insertEntry(void*, const RID&);
        bool deleteEntry(void*, const RID&);

    private:
        BufPageManager* bpm;
        int fileId;
        int attrLen, fanOut;
        
        BPlusNode cur;
        function<bool(void*, void*)> cmp;

        void traceToLeaf(void*);
};

#endif