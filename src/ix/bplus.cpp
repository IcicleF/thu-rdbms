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

void BPlusTree::deleteInnerNode(int pageId)
{
    if (pageId == root()) return;
    BPlusNode tcur,fa,lsib,rsib;
    tcur.owner = this;
    tcur.id = pageId;
    tcur.getPage();
    if (tcur.count() >= fanOut/2 - 1) return;
    int faid,fa_pos;
    faid = tcur.parent();
    fa_pos = tcur.parentPtr();
    fa.owner = this;
    fa.id = faid;
    fa.getPage();
    bool pl,pr;
    pl = false;
    pr = false;
    if (fa_pos != 0){
        lsib.pageId = fa.child(fa_pos - 1);
        lsib.owner = this;
        lsib.getPage();
        if (lsib.count() >= fanOut/2) pl = true;
    }
    else{
        rsib.pageId = fa.child(fa_pos + 1);
        rsib.owner = this;
        rsib.getPage();
        if (rsib.count() >= fanOut/2) pr = true;
    }

    if (pl == true || pr == true){
        if (pl == true){
            for (int i = tcur.count() - 1; i >= 0 ; i--)tcur.setBlock(i+1, tcur.block(i));
            for (int i = tcur.count(); i >= 0; i--)tcur.setChild(i+1, tcur.child(i));
            tcur.setChild(0, lsib.child(lsib.count()));
            tcur.setCount(tcur.count() + 1);
            tcur.setBlock(0, fa.block(fa_pos - 1));
            fa.setBlock(fa_pos - 1, lsib.block(lsib.count() - 1));
            lsib.setCount(lsib.count() - 1);
        }
        else {
            tcur.setBlock(tcur.count(), fa.block(fa_pos));
            tcur.setChild(tcur.count() + 1, rsib.child(0));
            tcur.setCount(tcus.count() + 1);
            fa.setBlock(fa_pos, rsib.block(0));
            for(int i = 0; i < rsib.count(); i++)rsib.setChild(i, rsib.child(i+1));
            for(int i = 1; i < rsib.count(); i++)rsib.setBlock(i-1, rsib.block(i));
            rsib.setCount(rsib.count() - 1);
        }
    }
    else {
        if (fa_pos != 0){
            for(int i = tcur.count() - 1 ; i >= 0 ; i--)tcur.setBlock(i + lsib.count() + 1, tcur.block(i));
            for(int i = tcur.count(); i >= 0 ; i--)tcur.setChild(i + lsib.count() + i, tcur.child(i));
            for(int i = 0; i <= lsib.count(); i++)tcur.setChild(i, lsib.child(i));
            for(int i = 0; i < lsib.count(); i++)tcur.setBlock(i, lsib.block(i));
            tcur.setBlock(lsib.count(), fa.block(fa_pos - 1));
            tcur.setCount(tcur.count() + lsib.count() + 1);
            for(int i = fa_pos; i < fa.count(); i++)fa.setBlock(i-1, fa.block(i));
            for(int i = fa_pos; i < fa.count(); i++)fa.setChild(i-1, fa.child(i));
            fa.setCount(fa.count() - 1);
            bpm->release(lsib.pageId);
        }
        else {
            for(int i = 0; i < rsib.count(); i++)tcur.setBlock(tcur.count() + 1 + i, rsib.block(i));
            for(int i = 0; i <= rsib.count(); i++)tcur.setChild(tcur.count() + 1 + i, rsib.child(i));
            tcur.setBlock(tcur.count(), fa.block(fa_pos));
            tcur.setCount(tcur.count() + rsib.count() + 1);
            for(int i = fa_pos + 1; i < fa.count(); i++)fa.setBlock(i-1, fa.block(i));
            for(int i = fa_pos + 1; i < fa.count(); i++)fa.setChild(i-1, fa.child(i));
            fa.setCount(fa.count() - 1);
            bpm->release(rsib.pageId);  
        }
        deleteInnerNode(faid);
    }
}

bool BPlusTree::deleteEntry(void *pData, const RID& rid) 
{
    RID rs;
    if(!searchEntry(pData, rs)){
        return false;
    }
    else{
        int l,faid,fa_pos;
        if (cur.pageId != root()){
            faid = cur.parent();
            fa_pos = cur.parentPtr();
        }
        l = cur.count();

        //删除操作
        int pos = 0;
        for (int i = 0 ; i < l ; i++){
            if (cmp(pData, cur.val(i)) == 0){
                pos = i;
                break;
            }
        }
        for (int i = pos + 1; i < l; i++)cur.setBlock(i, cur.block(i+1));
        cur.setCount(l - 1);
        BPlusNode fa;
        if (cur.pageId != root()){
            fa.pageId = faid;
            fa.owner = this;
            fa.getPage();
            if (pos == 0) fa.setVal(fa_pos - 1, fa.val(1));
        }

        if (l == fanOut/2 - 1 && cur.pageId != root()){
            // 删除的特殊处理
            bool pl,pr;
            pl = false;
            pr = false;
            BPlusNode lsib,rsib;
            lsib.owner = this;
            rsib.owner = this;
            if (fa_pos > 0){
                lsib.pageId = fa.child(fa_pos - 1);
                lsib.getPage();
                if (lsib.count() >= fanOut/2)pl = true;
            }
            if (fa_pos < l){
                rsib.pageId = fa.child(fa_pos + 1);
                rsib.getPage();
                if (rsib.count() >= fanOut/2)pr = true;
            }
            if(pl == true || pr == true){
                //直接由兄弟节点借
                if (pl == true){
                    fa.setVal(fa_pos - 1, lsib.val(lsib.count() - 1));
                    cur.setCount(l+1);
                    for(int i = l - 1; i >= 0; i--)cur.setBlock(i+1, cur.block(i));
                    cur.setBlock(0, lsib.block(lsib.count()-1));
                    lsib.setCount(lsib.count() - 1);
                }
                else{
                    fa.setVal(fa_pos, rsib.val(1));
                    cur.setCount(l+1);
                    cur.setBlock(l+1, rsib.block(0));
                    for(int i = 1; i < rsib.count(); i++)rsib.setBlock(i - 1, rsib.block(i));
                    rsib.setCount(rsib.count() - 1);
                }
            }
            else{
                //和兄弟节点合并，向父节点传递
                if (fa_pos != 0){
                    cur.setCount(lsib.count() + cur.count());
                    for(int i = cur.count() - 1; i >= 0; i--)cur.setBlock(i + lsib.count(), cur.block(i));
                    for(int i = 0; i < lsib.count(); i++)cur.setBlock(i, lsib.block(i));
                    for(int i = fa_pos; i < fa.count(); i++)fa.setBlock(i-1, fa.block(i));
                    for(int i = fa_pos; i < fa.count(); i++)fa.setChild(i-1, fa.child(i));
                    fa.setCount(fa.count() - 1);
                    bpm->release(lsib.pageId);
                }
                else{
                    cur.setCount(rsib.count() + cur.count());
                    for(int i = 0; i < rsib.count(); i++)cur.setBlock(i + cur.count(), rsib.block(i));
                    for(int i = fa_pos + 1; i < fa.count(); i++)fa.setBlock(i-1, fa.block(i));
                    for(int i = fa_pos + 1; i < fa.count(); i++)fa.setChild(i-1, fa.child(i));
                    fa.setCount(fa.count() - 1);
                    bpm->release(rsib.pageId);
                }
                deleteInnerNode(faid);
            }
        }
    }
}