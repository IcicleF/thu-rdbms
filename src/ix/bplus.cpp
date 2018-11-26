#include "ix/bplus.h"
#include <cassert>
#include <queue>
#include <iostream>
#include <iomanip>

using namespace std;

BPlusTree::BPlusTree() {
    cur.owner = this;
}

BPlusTree::BPlusTree(BufPageManager* bpm, int fileId) {
    this->bpm = bpm;
    this->fileId = fileId;

    p0 = bpm->getPage(fileId, 0, p0Index);
    this->attrLen = p0[0];
    //fanOut = (PAGE_SIZE - 16) / (attrLen + 6) + 1;
    fanOut = 4;             // 测试用！！！
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

// insert at leaf
void BPlusTree::insertEntry(void* pData, const RID& rid) {
    traceToLeaf(pData);
    int l = cur.count();
    if (l + 1 == fanOut) {      // node is full
        cout << "  insertEntry: full! need to split" << endl;
        // build oversized node
        CharBufType tbuf = new uchar[(attrLen + 6) * fanOut + 5];
        for (int i = 0, j = 0; i < fanOut; ++i) {
            int offs = (attrLen + 6) * i;
            if (i == j && (i == fanOut - 1 || cmp(pData, cur.val(j)) < 0)) {
                *((int*)(tbuf + offs)) = rid.getPage();
                *((short*)(tbuf + offs + 4)) = rid.getSlot();
                memcpy(tbuf + offs + 6, pData, attrLen);
            }
            else
                memcpy(tbuf + offs, cur.block(j++), attrLen + 6);
        }

        int n = nodeNum();
        setNodeNum(++n);

        BPlusNode nv(n);
        nv.owner = this;
        nv.getPage();
        nv.setCount(fanOut / 2);
        nv.setType(BT_LEAF);

        int curNext = cur.child(cur.count());

        cur.clear();
        cur.setCount(fanOut / 2);

        for (int i = 0; i < fanOut / 2; ++i)
            cur.setBlock(i, tbuf + (attrLen + 6) * i);
        cur.setChild(cur.count(), nv.pageId);

        for (int i = 0; i < fanOut / 2; ++i)
            nv.setBlock(i, tbuf + (attrLen + 6) * (i + fanOut / 2));
        nv.setChild(nv.count(), curNext);
        delete[] tbuf;

        // copy that key to parent
        CharBufType nextkey = new uchar[attrLen + 5];
        memcpy(nextkey, cur.val(0), attrLen);        
        assert((cur.parent() == 0) == (root() == cur.pageId));
        if (cur.parent() == 0) {
            makeRoot(nextkey, nv.pageId, cur.pageId);
            nv.setParent(root());
            cur.setParent(root());
        }
        else {
            nv.setParent(cur.parent());
            cur.pageId = cur.parent();
            cur.getPage();
            insertInner(nextkey, n);
        }
        delete[] nextkey;
    }
    else {
        cout << "  insertEntry: not full leaf" << endl;
        int pos = 0;        // pos 代表 pData 应该在的位置
        if (l > 0) {
            if (cmp(pData, cur.val(0)) >= 0)
                for (int j = l - 1; j >= 0; --j) {
                    cout << j << ": ";
                    if (cmp(pData, cur.val(j)) >= 0) {
                        pos = j + 1;
                        break;
                    }
                }
            cout << "  insertEntry: found pos = " << pos << endl;
            for (int j = l; j > pos; --j)
                cur.setBlock(j, cur.block(j - 1));
        }
        cur.clearBlock(pos);
        cur.setVal(pos, pData);
        cur.setRec(pos, rid);
        cur.setCount(cur.count() + 1);
    }
}

// insert at cur, cur must be inner node
void BPlusTree::insertInner(void* pData, int pageId) {
    assert(cur.type() == BT_INNER);

    int l = cur.count();
    if (l + 1 == fanOut) {      // node is full
        // build oversized node (ptr data separated)
        int* ptrs = new int[fanOut + 5];
        CharBufType tbuf = new uchar[attrLen * fanOut + 5];
        for (int i = 0, j = 0; i < fanOut; ++i) {
            int offs = attrLen * i;
            if (i == j && (i == fanOut - 1 || cmp(pData, cur.val(j)) < 0)) {
                ptrs[i] = pageId;
                memcpy(tbuf + offs, pData, attrLen);
            }
            else {
                ptrs[i] = cur.child(j);
                memcpy(tbuf + offs, cur.val(j++), attrLen);
            }
        }
        ptrs[fanOut] = cur.child(fanOut - 1);

        int n = nodeNum();
        setNodeNum(++n);

        BPlusNode nv(n);
        nv.owner = this;
        nv.getPage();
        nv.setCount(fanOut / 2);
        nv.setType(BT_INNER);

        cur.clear();
        cur.setCount(fanOut / 2 - 1);

        for (int i = 0; i < fanOut / 2; ++i) {
            nv.setChild(i, ptrs[i]);
            nv.setVal(i, tbuf + attrLen * i);
        }
        nv.setChild(fanOut / 2, ptrs[fanOut / 2]);
        for (int i = 0; i < fanOut / 2 - 1; ++i) {
            int j = i + 1 + fanOut / 2;
            cur.setChild(i, ptrs[j]);
            cur.setVal(i, tbuf + attrLen * j);
        }
        cur.setChild(fanOut / 2 - 1, ptrs[fanOut]);

        // raise that key to parent
        CharBufType nextkey = new uchar[attrLen + 5];
        memcpy(nextkey, tbuf + attrLen * (fanOut / 2), attrLen); 

        delete[] ptrs;
        delete[] tbuf;
        
        assert((cur.parent() == 0) == (root() == cur.pageId));
        if (cur.parent() == 0) {
            makeRoot(nextkey, nv.pageId, cur.pageId);
            nv.setParent(root());
            cur.setParent(root());
        }
        else {
            nv.setParent(cur.parent());
            cur.pageId = cur.parent();
            cur.getPage();
            insertInner(nextkey, n);
        }
        delete[] nextkey;
    }
    else {
        int pos = 0;
        if (cmp(pData, cur.val(0)) >= 0)
            for (int j = l - 1; j >= 0; --j)
                if (cmp(pData, cur.val(j)) >= 0) {
                    pos = j + 1;
                    break;
                }
        cur.setChild(l + 1, cur.child(l));
        for (int j = l; j > pos; --j)
            cur.setBlock(j, cur.block(j - 1));
        cur.clearBlock(pos);
        cur.setVal(pos, pData);
        cur.setChild(pos, pageId);
        cur.setCount(cur.count() + 1);
    }
}

void BPlusTree::makeRoot(void* pData, int left, int right) {
    int n = nodeNum();
    setNodeNum(++n);

    BPlusNode nroot(n);
    nroot.owner = this;
    nroot.getPage();
    nroot.setCount(1);
    nroot.setType(BT_INNER);
    nroot.setParent(0);
    nroot.setParentPtr(0);
    
    nroot.setChild(0, left);
    nroot.setVal(0, pData);
    nroot.setChild(1, right);
    setRoot(n);
    return;
}

void BPlusTree::deleteInnerNode(int pageId)
{
    if (pageId == root()) return;
    BPlusNode tcur,fa,lsib,rsib;
    tcur.owner = this;
    tcur.pageId = pageId;
    tcur.getPage();
    if (tcur.count() >= (fanOut-1)/2 - 1) return;
    int faid,fa_pos;
    faid = tcur.parent();
    fa_pos = tcur.parentPtr();
    fa.owner = this;
    fa.pageId = faid;
    fa.getPage();
    bool pl,pr;
    pl = false;
    pr = false;
    if (fa_pos != 0){
        lsib.pageId = fa.child(fa_pos - 1);
        lsib.owner = this;
        lsib.getPage();
        if (lsib.count() >= (fanOut-1)/2) pl = true;
    }
    else{
        rsib.pageId = fa.child(fa_pos + 1);
        rsib.owner = this;
        rsib.getPage();
        if (rsib.count() >= (fanOut-1)/2) pr = true;
    }

    if (pl == true || pr == true){
        if (pl == true){
            tcur.setChild(tcur.count() + 1, tcur.child(tcur.count()));
            for (int i = tcur.count() - 1; i >= 0 ; i--)tcur.setBlock(i+1, tcur.block(i));
            tcur.setChild(0, lsib.child(lsib.count()));
            tcur.setVal(0, fa.val(fa_pos - 1));
            tcur.setCount(tcur.count() + 1);
            fa.setVal(fa_pos - 1, lsib.val(lsib.count() - 1));
            lsib.setCount(lsib.count() - 1);
        }
        else {
            tcur.setVal(tcur.count(), fa.val(fa_pos));
            tcur.setChild(tcur.count() + 1, rsib.child(0));
            tcur.setCount(tcur.count() + 1);
            fa.setVal(fa_pos, rsib.val(0));
            for(int i = 1; i < rsib.count(); i++)rsib.setBlock(i-1, rsib.block(i));
            rsib.setChild(rsib.count() - 1, rsib.child(rsib.count()));
            rsib.setCount(rsib.count() - 1);
        }
    }
    else {
        this->setNodeNum(this->nodeNum() - 1);
        if (fa_pos != 0){
            tcur.setChild(tcur.count() + lsib.count() + 1, tcur.child(tcur.count()));
            for(int i = tcur.count() - 1 ; i >= 0 ; i--)tcur.setBlock(i + lsib.count() + 1, tcur.block(i));
            for(int i = 0; i < lsib.count(); i++)tcur.setBlock(i, lsib.block(i));
            tcur.setChild(lsib.count(), lsib.child(lsib.count()));
            tcur.setVal(lsib.count(), fa.val(fa_pos - 1));
            tcur.setCount(tcur.count() + lsib.count() + 1);
            bpm->release(lsib.pageId);
            if (faid == root() && fa.count() == 1){
                bpm->release(faid);
                this->setRoot(tcur.pageId);
                this->setNodeNum(this->nodeNum() - 1);
                return;
            }
            else{
                for(int i = fa_pos; i < fa.count(); i++)fa.setBlock(i-1, fa.block(i));
                fa.setChild(fa.count() - 1, fa.child(fa.count()));
                fa.setCount(fa.count() - 1);
            }
        }
        else {
            rsib.setChild(tcur.count() + rsib.count() + 1, rsib.child(rsib.count()));
            for(int i = rsib.count() - 1 ; i >= 0 ; i--)tcur.setBlock(i + tcur.count() + 1, tcur.block(i));
            for(int i = 0; i < tcur.count(); i++)rsib.setBlock(i, tcur.block(i));
            rsib.setChild(tcur.count(), tcur.child(tcur.count()));
            rsib.setVal(tcur.count(), fa.val(fa_pos));
            rsib.setCount(tcur.count() + rsib.count() + 1);
            bpm->release(tcur.pageId);
            if (faid == root() && fa.count() == 1){
                bpm->release(faid);
                this->setRoot(rsib.pageId);
                this->setNodeNum(this->nodeNum() - 1);
                return;
            }
            else{
                for(int i = fa_pos + 1; i < fa.count(); i++)fa.setBlock(i-1, fa.block(i));
                fa.setChild(fa.count() - 1, fa.child(fa.count()));
                fa.setCount(fa.count() - 1);
            }

        }
        deleteInnerNode(faid);
    }
}

bool BPlusTree::deleteEntry(void *pData, const RID& rid) 
{
    RID rs;
    BPlusNode tcur,fa,lsib,rsib;
    if(!searchEntry(pData, rs)){
        return false;
    }
    else{
        tcur.pageId = cur.pageId;
        tcur.owner = this;
        tcur.getPage();
        int faid,fa_pos;
        if (tcur.pageId != root()){
            faid = tcur.parent();
            fa_pos = tcur.parentPtr();
        }

        //删除操作
        int pos = 0;
        for (int i = 0 ; i < tcur.count() ; i++){
            if (cmp(pData, cur.val(i)) == 0){
                pos = i;
                break;
            }
        }
        for (int i = pos + 1; i < tcur.count(); i++)tcur.setBlock(i-1, tcur.block(i));
        tcur.setChild(tcur.count() - 1, tcur.child(tcur.count()));
        tcur.setCount(tcur.count() - 1);
        if (tcur.pageId != root()){
            fa.pageId = faid;
            fa.owner = this;
            fa.getPage();
            if (pos == 0) fa.setVal(fa_pos - 1, tcur.val(0));
        }

        if (tcur.count() < (fanOut - 1)/2 && tcur.pageId != root()){
            // 删除的特殊处理
            bool pl,pr;
            pl = false;
            pr = false;
            lsib.owner = this;
            rsib.owner = this;
            if (fa_pos > 0){
                lsib.pageId = fa.child(fa_pos - 1);
                lsib.getPage();
                if (lsib.count() > (fanOut-1)/2)pl = true;
            }
            if (fa_pos < fa.count()){
                rsib.pageId = fa.child(fa_pos + 1);
                rsib.getPage();
                if (rsib.count() > (fanOut-1)/2)pr = true;
            }
            if(pl == true || pr == true){
                //直接由兄弟节点借
                if (pl == true){
                    fa.setVal(fa_pos - 1, lsib.val(lsib.count() - 1));
                    cur.setChild(cur.count()+1, cur.child(cur.count()));
                    for(int i = cur.count() - 1; i >= 0; i--)cur.setBlock(i+1, cur.block(i));
                    cur.setCount(cur.count() + 1);
                    cur.setBlock(0, lsib.block(lsib.count()-1));
                    lsib.clearBlock(lsib.count() - 1);
                    lsib.setChild(lsib.count() - 1, lsib.count());
                    lsib.setCount(lsib.count() - 1);
                }
                else{
                    fa.setVal(fa_pos, rsib.val(1));
                    cur.setChild(cur.count() + 1, cur.child(cur.count()));
                    cur.setBlock(cur.count(), rsib.block(0));
                    cur.setCount(cur.count() + 1);
                    for(int i = 1; i < rsib.count(); i++)rsib.setBlock(i - 1, rsib.block(i));
                    rsib.clearBlock(rsib.count() - 1);
                    rsib.setChild(rsib.count() - 1, rsib.child(rsib.count()));
                    rsib.setCount(rsib.count() - 1);
                }
            }
            else{
                this->setNodeNum(this->nodeNum() - 1);
                //和兄弟节点合并，向父节点传递
                if (fa_pos != 0){
                    for (int i = 0; i < tcur.count(); i++)lsib.setBlock(i + lsib.count(), tcur.block(i));
                    lsib.setChild(tcur.count() + lsib.count(), tcur.child(tcur.count()));
                    lsib.setCount(tcur.count() + lsib.count());
                    bpm->release(tcur.pageId);
                    if (faid == root() && fa.count() == 1){
                        bpm->release(faid);
                        this->setRoot(lsib.pageId);
                        this->setNodeNum(this->nodeNum() - 1);
                        return;
                    }
                    else{
                        if (fa_pos < fa.count())fa.setVal(fa_pos - 1, fa.val(fa_pos));
                        for(int i = fa_pos + 1; i < fa.count(); i++)fa.setBlock(i-1, fa.block(i));
                        fa.setChild(fa.count() - 1, fa.child(fa.count()));
                        fa.setCount(fa.count() - 1);
                    }
                }
                else{
                    for (int i = 0; i < rsib.count(); i++)tcur.setBlock(i + tcur.count(), rsib.block(i));
                    tcur.setChild(tcur.count() + rsib.count(), rsib.child(rsib.count()));
                    tcur.setCount(tcur.count() + tcur.count());
                    bpm->release(rsib.pageId);
                    if (faid == root() && fa.count() == 1){
                        bpm->release(faid);
                        this->setRoot(tcur.pageId);
                        this->setNodeNum(this->nodeNum() - 1);
                        return;
                    }
                    else{
                        if (fa_pos + 1 < fa.count())fa.setVal(fa_pos, fa.val(fa_pos + 1));
                        for(int i = fa_pos + 2; i < fa.count(); i++)fa.setBlock(i-1, fa.block(i));
                        fa.setChild(fa.count() - 1, fa.child(fa.count()));
                        fa.setCount(fa.count() - 1);
                    }
                }
                deleteInnerNode(faid);
            }
        }
    }
    return true;
}

void BPlusTree::printTree() {
    setiosflags(ios::showbase);

    queue<int> Q;
    Q.push(root());

    int pageId, pageIndex;
    while (!Q.empty()) {
        pageId = Q.front();
        Q.pop();

        BPlusNode v(pageId);
        v.owner = this;
        v.getPage();
        int l = v.count();

        cout << "=== Node #" << pageId << " ===" << endl;
        cout << " Type: " << (v.type() == BT_INNER ? "INNER" : "LEAF") << endl;
        cout << " Count: " << l << endl;
        cout << " Parent: ";
        if (v.parent() == 0)
            cout << "(NULL)" << endl;
        else
            cout << v.parent() << endl;

        cout << " Records:" << endl;
        if (v.type() == BT_LEAF)
            for (int i = 0; i < l; ++i) {
                RID rid = v.rec(i);
                cout << "   Pointer: Page #" << dec << rid.getPage() << ", Slot #" << rid.getSlot() << endl;
                cout << "   Rec: ";

                uchar* base = (uchar*)v.val(i);
                for (int j = 0; j < attrLen; ++j)
                    cout << "0x" << hex << (int)(*(base + j)) << " ";
                cout << endl;
            }
        else {
            for (int i = 0; i < l; ++i) {
                RID rid = v.rec(i);
                cout << "   Pointer: Page #" << v.child(i) << endl;
                cout << "   Rec: ";

                uchar* base = (uchar*)v.val(i);
                for (int j = 0; j < attrLen; ++j)
                    cout << "0x" << hex << (int)(*(base + j)) << " ";
                cout << endl;
                Q.push(v.child(i));
            }
            cout << "   Pointer: Page #" << v.child(l) << endl;
            Q.push(v.child(l));
        }
        cout << endl;
    }
}