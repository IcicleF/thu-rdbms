#ifndef BPLUS_H
#define BPLUS_H

#include <cstdlib>
#include <string>
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
        auto bpm = owner->bpm;
        int fileId = owner->fileId;
        if (fileId < 0 || pageId < 0)
            return;
        if (bpm == NULL || bpm == nullptr)
            return;
        page = bpm->getPage(fileId, pageId, pageIndex);
    }

    // call owner's cmp
    bool cmp(void* p1, void* p2) const { return owner->cmp(p1, p2); }

    // access attrLen from owner
    int getAttrLen() const { return owner->attrLen; }

    // get record count in this node
    short count() const { return spage[0]; }
    
    // get node type
    short type() const { return spage[1]; }

    // get parent node
    int parent() const { return ipage[1]; }

    // get parent ptr index to self
    int parentptr() const { return spage[4]; }

    // get child node #i (from #0)
    int child(int i) const {
        if (i < 0)
            return -1;
        if (i == 0)
            return ipage[2];
        else {
            int offs = 10 + (getAttrLen() + 10) * (i + 1);
            if (offs + 4 > PAGE_SIZE)
                return -1;
            int* ptr = (int*)(page + offs);
            return *ptr;
        }
    }

    // get address of attribute of record #i
    void* val(int i) const {
        if (i < 0)
            return NULL;
        int offs = 14 + (getAttrLen() + 10) * i;
        if (offs + getAttrLen() > PAGE_SIZE)
            return NULL;
        return (void*)(page + offs);
    }

    // return RID stored in rec #i
    RID rec(int i) const {
        if (i < 0)
            return RID();
        int offs = (getAttrLen() + 10) * (i + 1) + 4;
        if (offs + 6 > PAGE_SIZE)
            return RID();
        int* ptrPage = (int*)(page + offs);
        short* ptrSlot = (short*)(page + offs + 4);
        return RID(*ptrPage, (int)*ptrSlot);
    }
};

class BPlusTree {
    friend struct BPlusNode;

    public:
        BPlusTree();
        BPlusTree(BufPageManager*, int, int);
        ~BPlusTree();

        bool searchEntry(void*, RID&);
        bool insertEntry(void*, const RID&);

    private:
        BufPageManager* bpm;
        int fileId;
        int attrLen, fanOut;
        
        BPlusNode cur;
        function<bool(void*, void*)> cmp;

        void traceToLeaf(void*);
        void insertToCur(void*);
};

#endif