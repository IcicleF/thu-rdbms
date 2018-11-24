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
    if (l + 1 == fanOut) {
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
        nv.setParentPtr(0);             // todo!

        cur.setCount(fanOut / 2);

        for (int i = 0; i < fanOut / 2; ++i)
            nv.setBlock(i, tbuf + (attrLen + 6) * i);
        for (int i = 0; i < fanOut / 2; ++i)
            cur.setBlock(i, tbuf + (attrLen + 6) * (i + fanOut / 2));
        delete[] tbuf;

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
        int pos = 0;        // pos 代表 pData 应该在的位置
        if (l > 0) {
            if (cmp(pData, cur.val(0)) >= 0)
                for (int j = l - 1; j >= 0; --j)
                    if (cmp(pData, cur.val(j)) >= 0) {
                        pos = j + 1;
                        break;
                    }
            for (int j = l; j > pos; --j)
                cur.setBlock(j, cur.block(j - 1));
        }
        cur.setVal(pos, pData);
        cur.setRec(pos, rid);
        cur.setCount(cur.count() + 1);
    }
}

// insert at cur, cur must be inner node
void BPlusTree::insertInner(void* pData, int pageId) {
    assert(cur.type() == BT_INNER);

    int l = cur.count();
    if (l + 1 == fanOut) {
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
        nv.setParentPtr(0);             // todo!

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

        if (l == fanOut/2 && cur.pageId != root()){
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
                if (lsib.count() > fanOut/2)pl = true;
            }
            if (fa_pos < l){
                rsib.pageId = fa.child(fa_pos + 1);
                rsib.getPage();
                if (rsib.count() > fanOut/2)pr = true;
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
                if (fa_pos != 0){
                    cur.setCount(lsib.count() + cur.count());
                    for(int i = cur.count() - 1; i >= 0; i--)cur.setBlock(i + lsib.count(), cur.block(i));
                    for(int i = 0; i < lsib.count(); i++)cur.setBlock(i, lsib.block(i));
                    for(int i = fa_pos; i < fa.count(); i++)fa.setBlock(i-1, fa.block(i));
                    fa.setCount(fa.count() - 1);
                    bpm->release(lsib.pageId);
                }
                else{

                }
            }

        }
        else{
            //删除后不出现异常
        }
    }
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
        if (v.type() == BT_INNER)
            for (int i = 0; i < l; ++i) {
                RID rid = v.rec(i);
                cout << "   Pointer: Page #" << rid.getPage() << ", Slot #" << rid.getSlot() << endl;
                cout << "   Rec: ";

                uchar* base = (uchar*)v.val(i);
                for (int j = 0; j < attrLen; ++j)
                    cout << hex << (int)(*(base + j)) << " ";
                cout << endl;
            }
        else {
            for (int i = 0; i < l; ++i) {
                RID rid = v.rec(i);
                cout << "   Pointer: Page #" << v.child(i);
                cout << "   Rec: ";

                uchar* base = (uchar*)v.val(i);
                for (int j = 0; j < attrLen; ++j)
                    cout << hex << (int)(*(base + j)) << " ";
                cout << endl;
                Q.push(v.child(i));
            }
            cout << "   Pointer: Page #" << v.child(l);
            Q.push(v.child(l));
        }
        cout << endl;
    }
}