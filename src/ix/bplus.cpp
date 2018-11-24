#include "ix/bplus.h"

using namespace std;

BPlusTree::BPlusTree() {
    cur.owner = this;
}

BPlusTree::BPlusTree(BufPageManager* bpm, int fileId) {
    this->bpm = bpm;
    this->fileId = fileId;

    p0 = bpm->getPage(fileId, 0, p0Index);
    this->attrLen = p0[0];
    fanOut = (PAGE_SIZE - 16) / (attrLen + 6) + 1;
    if (fanOut & 1)
        fanOut -= 1;        // force even
    cur.owner = this;
}

BPlusTree::~BPlusTree() {
    // Nothing special.
}

void BPlusTree::traceToLeaf(void *pData) {
    // set cur as root
    cur.pageId = root();
    cur.getPage();

    while (cur.type() != BT_LEAF) {
        if (cmp(pData, cur.val(0)) < 0)
            cur.pageId = cur.child(0);
        else {
            short l = cur.count();
            for (int i = l - 1; i >= 0; --i)
                if (cmp(pData, cur.val(i)) >= 0) {
                    cur.pageId = cur.child(i + 1);
                    break;
                }
        }
        cur.getPage();
    }
}

bool BPlusTree::searchEntry(void* pData, RID& rid) {
    traceToLeaf(pData);
    int l = cur.count();
    for (int i = 0, r; i < l; ++i) {
        r = cmp(pData, cur.val(i));
        if (r > 0)
            break;
        if (r == 0) {
            rid = cur.rec(i);
            return true;
        } 
    }
    rid = RID();
    return false;
}

bool BPlusTree::insertEntry(void* pData, const RID& rid) {
    traceToLeaf(pData);
    int l = cur.count();
    if (l + 1 == fanOut) {
        // 叶子已满，分裂并调用 insertInner
        // 在这个时候叶子中有 fanOut - 1 条记录，再加一条新增的合计 fanOut 条
        // 由于 fanOut 是偶数，因此直接对半分

        // 1. new node
        int n = nodeNum();
        setNodeNum(++n);

        BPlusNode nv(n);
        nv.owner = this;
        nv.getPage();

        // 2. write metadata
        nv.setCount(fanOut / 2);
        nv.setType(BT_LEAF);
        nv.setParent(cur.parent());
        nv.setParentPtr(0);             // todo

        // 3. copy data
        for (int i = 0, j = 0; i < fanOut / 2; ++i) {
            
        }
    }
    else {
        int pos = 0;        // pos 代表 pData 应该在的位置
        if (cmp(pData, cur.val(0)) >= 0)
            for (int j = l - 1; j >= 0; --j)
                if (cmp(pData, cur.val(j)) >= 0) {
                    pos = j + 1;
                    break;
                }
        for (int j = l; j > pos; --j)
            cur.setBlock(j, cur.block(j - 1));
        cur.setVal(pos, pData);
        cur.setRec(pos, rid);
    }
}

bool BPlusTree::deleteEntry(void *pData, const RID& rid) 
{
    RID rs;
    if(!searchEntry(pData, rs)){
        return false;
    }
    else{
        l = cur.count();
        if (l == fanOut/2){
            // 删除的特殊处理
        }
        else{
            int pos = 0;
            for (int i = 0 ; i < l ; i++){
                if (cmp(pData, cur.val(i)) == 0){
                    pos = i;
                    break;
                }
            }
            if (pos == 0){
                int faid = cur.parent();
                int faord = cur.parentPtr();
                cur.pageID = faid;
                cur.getPage();
                
            }
            for (int i = pos + 1; i < l; i++)cur.copy(i, i+1);
            cur.setCount(l - 1);
        }
    }
}