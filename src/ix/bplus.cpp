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
    this->_nodeNum = p0[2];
    this->_root = p0[3];
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
        if (cmp(pData, cur.val(0)) < 0){
            cur.pageId = cur.child(0);
        }
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
    int par = cur.parent();
    if (l + 1 == fanOut) {      // node is full
        cout << "  insertEntry: full! need to split" << endl;
        // build oversized node
        CharBufType tbuf = new uchar[(attrLen + 6) * fanOut + 5];
        memset(tbuf, 0, (attrLen + 6) * fanOut + 5);
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

        int curNext = cur.child(cur.count());

        int n = nodeNum();
        setNodeNum(++n);
        
        // right node: splitted
        BPlusNode nv(n);
        nv.owner = this;
        nv.getPage();
        nv.clear();
        nv.setCount(fanOut / 2);
        nv.setType(BT_LEAF);
        for (int i = 0; i < fanOut / 2; ++i)
            nv.setBlock(i, tbuf + (attrLen + 6) * (i + fanOut / 2));
        nv.setChild(nv.count(), curNext);

        // copy that key to parent
        CharBufType nextkey = new uchar[attrLen + 5];
        memcpy(nextkey, nv.val(0), attrLen);

        // left node: cur
        cur.getPage();
        cur.clear();
        cur.setCount(fanOut / 2);
        for (int i = 0; i < fanOut / 2; ++i)
            cur.setBlock(i, tbuf + (attrLen + 6) * i);
        cur.setChild(cur.count(), nv.pageId);

        delete[] tbuf;

        assert((par == 0) == (root() == cur.pageId));
        if (par == 0) {
            makeRoot(nextkey, cur.pageId, nv.pageId);
            cur.setParent(root());
            nv.getPage();
            nv.setParent(root());
        }
        else {
            nv.getPage();
            nv.setParent(par);
            cur.pageId = par;
            cur.getPage();
            insertInner(nextkey, nv.pageId);
        }
        delete[] nextkey;
    }
    else {
        cout << "  insertEntry: not full leaf" << endl;
        int pos = 0;        // pos 代表 pData 应该在的位置
        if (l > 0) {
            if (cmp(pData, cur.val(0)) >= 0)
                for (int j = l - 1; j >= 0; --j) {
                    if (cmp(pData, cur.val(j)) >= 0) {
                        pos = j + 1;
                        break;
                    }
                }
            cout << "  insertEntry: found pos = " << pos << endl;
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
    cout << "  insertInner: " << (char*)pData << " " << pageId << endl;

    assert(cur.type() == BT_INNER);
    int l = cur.count();
    int par = cur.parent();
    if (l + 1 == fanOut) {      // node is full
        // build oversized node (ptr data separated)
        int* ptrs = new int[fanOut + 5];
        CharBufType tbuf = new uchar[attrLen * fanOut + 5];
        memset(tbuf, 0, attrLen * fanOut + 5);
        ptrs[0] = cur.child(0);
        for (int i = 1, j = 0; i <= fanOut; ++i) {
            int offs = attrLen * i;
            if (i == j + 1 && (i == fanOut || cmp(pData, cur.val(j)) < 0)) {
                ptrs[i] = pageId;
                memcpy(tbuf + offs, pData, attrLen);
            }
            else {
                ptrs[i] = cur.child(j);
                memcpy(tbuf + offs, cur.val(j++), attrLen);
            }
        }

        cout << "  insertInner: splitting" << endl;
        cout << "  "; for(int i = 0; i <= fanOut; ++i) cout << dec << ptrs[i] << " ";
        cout << endl;

        int n = nodeNum();
        setNodeNum(++n);

        // left node: cur
        cur.getPage();
        cur.clear();
        cur.setCount(fanOut / 2);
        for (int i = 0; i < fanOut / 2; ++i) {
            cur.setChild(i, ptrs[i]);
            cur.setVal(i, tbuf + attrLen * i);
        }
        cur.setChild(fanOut / 2, ptrs[fanOut / 2]);

        // right node: splitted
        BPlusNode nv(n);
        nv.owner = this;
        nv.getPage();
        nv.clear();
        nv.setCount(fanOut / 2 - 1);
        nv.setType(BT_INNER);
        for (int i = 0; i < fanOut / 2 - 1; ++i) {
            int j = i + 1 + fanOut / 2;
            nv.setChild(i, ptrs[j]);
            nv.setVal(i, tbuf + attrLen * j);
        }
        nv.setChild(fanOut / 2 - 1, ptrs[fanOut]);

        // raise that key to parent
        CharBufType nextkey = new uchar[attrLen + 5];
        memcpy(nextkey, tbuf + attrLen * (fanOut / 2), attrLen); 

        delete[] ptrs;
        delete[] tbuf;
        
        assert((par == 0) == (root() == cur.pageId));
        if (par == 0) {
            makeRoot(nextkey, cur.pageId, nv.pageId);
            nv.setParent(root());
            cur.getPage();
            cur.setParent(root());
        }
        else {
            nv.setParent(par);
            cur.pageId = par;
            cur.getPage();
            insertInner(nextkey, nv.pageId);
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
        cout << "  insertEntry: found pos = " << pos << endl;
        cur.setChild(l + 1, cur.child(l));
        for (int j = l; j > pos; --j)
            cur.setBlock(j, cur.block(j - 1));
        cur.setVal(pos, pData);
        cur.setChild(pos + 1, pageId);
        cur.setCount(cur.count() + 1);
    }
}

void BPlusTree::makeRoot(void* pData, int left, int right) {
    cout << "  makeRoot: " << (char*)pData << " " << left << " " << right << endl;

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
    BPlusNode tcur,fa,lsib,rsib,child;
    BPlusNode temp;
    short tempcount;
    temp.owner = this;
    tcur.owner = this;
    tcur.pageId = pageId;
    tcur.getPage();
    if (tcur.count() >= (fanOut-1)/2 ) return;
    int faid,fa_pos;
    faid = tcur.parent();
    fa.owner = this;
    fa.pageId = faid;
    fa.getPage();
    
    child.owner = this;

    for (int i = 0; i <= fa.count(); i++){
        if (fa.child(i) == tcur.pageId){
            fa_pos = i;
            break;
        }
    }

    bool pl,pr;
    pl = false;
    pr = false;
    fa.getPage();
    if (fa_pos != 0){
        lsib.pageId = fa.child(fa_pos - 1);
        lsib.owner = this;
        lsib.getPage();
        if (lsib.count() > (fanOut-1)/2) pl = true;
    }
    else{
        rsib.pageId = fa.child(fa_pos + 1);
        rsib.owner = this;
        rsib.getPage();
        if (rsib.count() > (fanOut-1)/2) pr = true;
    }

    if (pl == true || pr == true){
        if (pl == true){
            lsib.getPage();
            memcpy(temp.page, lsib.page, PAGE_SIZE);//temp = lsib
            tcur.getPage();
            tcur.setChild(tcur.count() + 1, tcur.child(tcur.count()));
            for (int i = tcur.count() - 1; i >= 0 ; i--)tcur.setBlock(i+1, tcur.block(i));
            tcur.setChild(0, temp.child(temp.count()));
            fa.getPage();
            memcpy(temp.page, fa.page, PAGE_SIZE);//temp = fa
            tcur.getPage();
            tcur.setVal(0, temp.val(fa_pos - 1));
            tcur.setCount(tcur.count() + 1);
            lsib.getPage();
            memcpy(temp.page, lsib.page, PAGE_SIZE);//temp = lsib
            fa.getPage();
            fa.setVal(fa_pos - 1, temp.val(temp.count() - 1));
            lsib.getPage();
            lsib.setCount(lsib.count() - 1);
        }
        else {
            fa.getPage();
            memcpy(temp.page, fa.page, PAGE_SIZE);//temp = fa
            tcur.getPage();
            tcur.setVal(tcur.count(), temp.val(fa_pos));
            rsib.getPage();
            memcpy(temp.page, rsib.page, PAGE_SIZE);//temp = rsib
            tcur.getPage();
            tcur.setChild(tcur.count() + 1, temp.child(0));
            tcur.setCount(tcur.count() + 1);
            fa.getPage();
            fa.setVal(fa_pos, temp.val(0));
            rsib.getPage();
            for(int i = 1; i < rsib.count(); i++)rsib.setBlock(i-1, rsib.block(i));
            rsib.setChild(rsib.count() - 1, rsib.child(rsib.count()));
            rsib.setCount(rsib.count() - 1);
        }
    }
    else {
        this->setNodeNum(this->nodeNum() - 1);
        if (fa_pos != 0){
            lsib.getPage();
            memcpy(temp.page, lsib.page, PAGE_SIZE);//temp = lsib
            tcur.getPage();
            tcur.setChild(tcur.count() + temp.count() + 1, tcur.child(tcur.count()));
            for(int i = 0; i <= temp.count(); i++){
                child.pageId = temp.child(i);
                child.getPage();
                child.setParent(tcur.pageId);
            }
            for(int i = tcur.count() - 1 ; i >= 0 ; i--)tcur.setBlock(i + temp.count() + 1, tcur.block(i));
            for(int i = 0; i < temp.count(); i++)tcur.setBlock(i, temp.block(i));
            tcur.setChild(temp.count(), temp.child(temp.count()));
            tempcount = temp.count();//tempcount = lsib.count
            fa.getPage();
            memcpy(temp.page, fa.page, PAGE_SIZE);//temp = fa
            tcur.getPage();
            tcur.setVal(tempcount, temp.val(fa_pos - 1));
            tcur.setCount(tcur.count() + tempcount + 1);
            bpm->release(lsib.pageId);
            if (faid == root() && fa.count() == 1){
                bpm->release(faid);
                this->setRoot(tcur.pageId);
                this->setNodeNum(this->nodeNum() - 1);
                return;
            }
            else{
                fa.getPage();
                for(int i = fa_pos; i < fa.count(); i++)fa.setBlock(i-1, fa.block(i));
                fa.setChild(fa.count() - 1, fa.child(fa.count()));
                fa.setCount(fa.count() - 1);
            }
        }
        else {
            tcur.getPage();
            memcpy(temp.page, tcur.page, PAGE_SIZE);//temp = tcur
            rsib.getPage();
            rsib.setChild(temp.count() + rsib.count() + 1, rsib.child(rsib.count()));
            for(int i = 0; i <= temp.count(); i++){
                child.pageId = temp.child(i);
                child.getPage();
                child.setParent(rsib.pageId);
            }
            for(int i = rsib.count() - 1 ; i >= 0 ; i--)rsib.setBlock(i + temp.count() + 1, rsib.block(i));
            for(int i = 0; i < temp.count(); i++)rsib.setBlock(i, temp.block(i));
            rsib.setChild(temp.count(), temp.child(temp.count()));
            tempcount = tcur.count(); //tempcount = tcur.count
            fa.getPage();
            memcpy(temp.page, fa.page, PAGE_SIZE);//temp = fa
            rsib.getPage();
            rsib.setVal(tempcount, temp.val(fa_pos));
            rsib.setCount(tempcount + rsib.count() + 1);
            bpm->release(tcur.pageId);
            if (faid == root() && fa.count() == 1){
                bpm->release(faid);
                this->setRoot(rsib.pageId);
                this->setNodeNum(this->nodeNum() - 1);
                return;
            }
            else{
                fa.getPage();
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
    BPlusNode tcur,fa,lsib,rsib,child;
    BPlusNode temp;
    temp.owner = this;
    temp.page = new unsigned char[PAGE_SIZE + 20];
    if(!searchEntry(pData, rs)){
        return false;
    }
    else{
        child.owner = this;

        tcur.pageId = cur.pageId;
        tcur.owner = this;
        tcur.getPage();
        int faid,fa_pos;
        if (tcur.pageId != root()){
            faid = tcur.parent();
            fa.pageId = faid;
            fa.owner = this;
            fa.getPage();
            for (int i = 0; i <= fa.count(); i++){
                if (fa.child(i) == tcur.pageId){
                    fa_pos = i;
                    break;
                }
            }
        }
        cout << "Delete " << (char *)pData << endl;
        //删除操作
        tcur.getPage();
        int pos = 0;
        for (int i = 0 ; i < tcur.count() ; i++){
            if (cmp(pData, tcur.val(i)) == 0){
                pos = i;
                break;
            }
        }
        for (int i = pos + 1; i < tcur.count(); i++)tcur.setBlock(i-1, tcur.block(i));
        tcur.setChild(tcur.count() - 1, tcur.child(tcur.count()));
        tcur.setCount(tcur.count() - 1);
        if (tcur.pageId != root()){
            fa.getPage();
            if (pos == 0) fa.setVal(fa_pos - 1, tcur.val(0));
        }

        if (tcur.count() < (fanOut - 1)/2 && tcur.pageId != root()){
            cout << "delete special" << endl;
            // 删除的特殊处理
            bool pl,pr;
            pl = false;
            pr = false;
            lsib.owner = this;
            rsib.owner = this;
            fa.getPage();
            if (fa_pos > 0){
                lsib.pageId = fa.child(fa_pos - 1);
                lsib.getPage();
                if (lsib.count() > (fanOut-1)/2)pl = true;
            }
            fa.getPage();
            if (fa_pos < fa.count()){
                rsib.pageId = fa.child(fa_pos + 1);
                rsib.getPage();
                if (rsib.count() > (fanOut-1)/2)pr = true;
            }
            if(pl == true || pr == true){
                //直接由兄弟节点借
                if (pl == true){
                    lsib.getPage();
                    memcpy(temp.page, lsib.page, PAGE_SIZE);//temp = lsib
                    fa.getPage();
                    fa.setVal(fa_pos - 1, temp.val(temp.count() - 1));
                    tcur.getPage();
                    tcur.setChild(tcur.count()+1, tcur.child(tcur.count()));
                    for(int i = tcur.count() - 1; i >= 0; i--)tcur.setBlock(i+1, tcur.block(i));
                    tcur.setCount(tcur.count() + 1);
                    tcur.setBlock(0, temp.block(temp.count()-1));
                    lsib.getPage();
                    lsib.clearBlock(lsib.count() - 1);
                    lsib.setChild(lsib.count() - 1, lsib.count());
                    lsib.setCount(lsib.count() - 1);
                }
                else{
                    rsib.getPage();
                    memcpy(temp.page, rsib.page, PAGE_SIZE);//temp = rsib;
                    fa.getPage();
                    fa.setVal(fa_pos, temp.val(1));
                    tcur.getPage();
                    tcur.setChild(tcur.count() + 1, tcur.child(tcur.count()));
                    tcur.setBlock(tcur.count(), temp.block(0));
                    tcur.setCount(tcur.count() + 1);
                    rsib.getPage();
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
                    tcur.getPage();
                    memcpy(temp.page, tcur.page ,PAGE_SIZE);//temp = tcur
                    lsib.getPage();
                    for (int i = 0; i < temp.count(); i++)lsib.setBlock(i + lsib.count(), temp.block(i));
                    lsib.setChild(temp.count() + lsib.count(), temp.child(temp.count()));
                    lsib.setCount(temp.count() + lsib.count());
                    bpm->release(tcur.pageId);
                    if (faid == root() && fa.count() == 1){
                        bpm->release(faid);
                        this->setRoot(lsib.pageId);
                        lsib.setParent(0);
                        this->setNodeNum(this->nodeNum() - 1);
                        return true;
                    }
                    else{
                        fa.getPage();
                        if (fa_pos < fa.count())fa.setVal(fa_pos - 1, fa.val(fa_pos));
                        for (int i = fa_pos + 1; i < fa.count(); i++)fa.setBlock(i-1, fa.block(i));
                        fa.setChild(fa.count() - 1, fa.child(fa.count()));
                        fa.setCount(fa.count() - 1);
                    }
                }
                else{
                    rsib.getPage();
                    memcpy(temp.page, rsib.page, PAGE_SIZE);//temp = rsib
                    tcur.getPage();
                    for (int i = 0; i < temp.count(); i++)tcur.setBlock(i + tcur.count(), temp.block(i));
                    tcur.setChild(tcur.count() + temp.count(), temp.child(temp.count()));
                    tcur.setCount(tcur.count() + temp.count());
                    bpm->release(rsib.pageId);
                    if (faid == root() && fa.count() == 1){
                        bpm->release(faid);
                        this->setRoot(tcur.pageId);
                        tcur.setParent(0);
                        this->setNodeNum(this->nodeNum() - 1);
                        return true;
                    }
                    else{
                        fa.getPage();
                        if (fa_pos + 1 < fa.count())fa.setVal(fa_pos, fa.val(fa_pos + 1));
                        for (int i = fa_pos + 2; i < fa.count(); i++)fa.setBlock(i-1, fa.block(i));
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
        if (v.type() == BT_LEAF){
            for (int i = 0; i < l; ++i) {
                RID rid = v.rec(i);
                cout << "   Pointer: Page #" << dec << rid.getPage() << ", Slot #" << rid.getSlot() << endl;
                cout << "   Rec: ";

                uchar* base = (uchar*)v.val(i);
                for (int j = 0; j < attrLen; ++j)
                    cout << "0x" << hex << (int)(*(base + j)) << " ";
                cout << endl;
            }
            cout << "   Next Page : Page #" << v.child(l) << endl;
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