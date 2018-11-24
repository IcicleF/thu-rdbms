#ifndef BPLUSNODE_H
#define BPLUSNODE_H

#include <cstdlib>
#include <cstring>
#include "pf/bufmanager/BufPageManager.h"
#include "rid.h"

class BPlusTree;

struct BPlusNode {
    BPlusTree* owner;
    int pageId, pageIndex;
    union {
        CharBufType page;
        ShortBufType spage;
        BufType ipage;
    };

    BPlusNode(int pageId = -1) : pageId(pageId) { }

    // load page from current pageId
    void getPage();

    // access attrLen from owner
    int getAttrLen() const;

    // access record count in this node
    short count() const;
    void setCount(short _count);
    
    // access node type
    short type() const;
    void setType(short _type);

    // access parent node
    int parent() const;
    void setParent(int _p);

    // access parent ptr index to self
    short parentPtr() const;
    void setParentPtr(short _pp);

    // access address of combinition of record #i
    void* block(int i);
    void setBlock(int i, void* pbData);

    // access address of attribute of record #i
    void* val(int i) const;
    void setVal(int i, void* pData);

    // access child node #i (from #0)
    int child(int i) const;
    void setChild(int i, int ch);

    // access last pointer using owner's fanOut (should be used in leaf only)
    int lastPtr() const;
    void setLastPtr(int p);

    // access RID stored in rec #i
    RID rec(int i) const;
    void setRec(int i, const RID& rid);
};

#endif