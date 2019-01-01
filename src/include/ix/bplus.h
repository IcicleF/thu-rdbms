#ifndef BPLUS_H
#define BPLUS_H

#include <set>
#include <functional>
#include "ix/bplusnode.h"

const short BT_INNER = 1, BT_LEAF = 2;

class IXScanner;

class BPlusTree {
    friend struct BPlusNode;
    friend class IXScanner;

    public:
        std::set<int> usedPages;
        std::function<int(void*, void*)> cmp;
        BufPageManager* bpm;

    public:
        BPlusTree();
        BPlusTree(BufPageManager*, int);
        ~BPlusTree();

        bool searchEntry(void*, RID&);
        void insertEntry(void*, const RID&);
        bool deleteEntry(void*, const RID&);

        void printTree();

    private:
        int fileId;
        int attrLen, fanOut;

        BufType p0;
        int p0Index;
        int _nodeNum, _root;
        int nodeNum() { return _nodeNum; }
        int root() { return _root; }
        void setNodeNum(int nn) {
            bpm->getPage(fileId, 0, p0Index);
            p0[2] = _nodeNum = nn;
            bpm->markDirty(p0Index);
            bpm->writeBack(p0Index);
        }
        void setRoot(int r) {
            bpm->getPage(fileId, 0, p0Index);
            p0[3] = _root = r;
            bpm->markDirty(p0Index);
            bpm->writeBack(p0Index);
        }
        
        BPlusNode cur;

        void traceToLeaf(void*);
        void insertInner(void*, int);
        void makeRoot(void*, int, int);
        void deleteInnerNode(int);
};

#endif