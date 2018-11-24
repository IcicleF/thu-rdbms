#ifndef BPLUS_H
#define BPLUS_H

#include <functional>
#include "ix/bplusnode.h"

const short BT_INNER = 1, BT_LEAF = 2;

class BPlusTree {
    friend struct BPlusNode;

    public:
        function<bool(void*, void*)> cmp;

    public:
        BPlusTree();
        BPlusTree(BufPageManager*, int);
        ~BPlusTree();

        bool searchEntry(void*, RID&);
        void insertEntry(void*, const RID&);
        bool deleteEntry(void*, const RID&);

        void printTree();

    private:
        BufPageManager* bpm;
        int fileId;
        int attrLen, fanOut;

        BufType p0;
        int p0Index;
        int nodeNum() { return p0[2]; }
        int root() { return p0[3]; }
        void setNodeNum(int nn) {
            p0[2] = nn;
            bpm->markDirty(p0Index);
        }
        void setRoot(int r) {
            p0[3] = r;
            bpm->markDirty(p0Index);
        }
        
        BPlusNode cur;

        void traceToLeaf(void*);
        void insertInner(void*, int);
        void makeRoot(void*, int, int);
        void deleteInnerNode(int);
};

#endif