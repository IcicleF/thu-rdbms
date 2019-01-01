#include "ix/bplus.h"
#include "ix/bplusnode.h"

using namespace std;

void BPlusNode::getPage() {
    page = (CharBufType)owner->bpm->getPage(owner->fileId, pageId, pageIndex);
    owner->usedPages.insert(pageIndex);
}
void BPlusNode::clear() {
    memset(page + 10, 0, PAGE_SIZE - 10);
    owner->bpm->markDirty(pageIndex);
    //owner->bpm->writeBack(pageIndex);
}

int BPlusNode::getAttrLen() const { 
    return owner->attrLen;
}

short BPlusNode::count() const { 
    return spage[0]; 
}
void BPlusNode::setCount(short _count) {
    spage[0] = _count;
    owner->bpm->markDirty(pageIndex);
    //owner->bpm->writeBack(pageIndex);
}

short BPlusNode::type() const { 
    return spage[1]; 
}
void BPlusNode::setType(short _type) {
    spage[1] = _type;
    owner->bpm->markDirty(pageIndex);
    //owner->bpm->writeBack(pageIndex);
}

int BPlusNode::parent() const { 
    return ipage[1]; 
}
void BPlusNode::setParent(int _p) {
    ipage[1] = _p;
    owner->bpm->markDirty(pageIndex);
    //owner->bpm->writeBack(pageIndex);
}

short BPlusNode::parentPtr() const { 
    return spage[4]; 
}
void BPlusNode::setParentPtr(short _pp) {
    spage[4] = _pp;
    owner->bpm->markDirty(pageIndex);
    //owner->bpm->writeBack(pageIndex);
}

void* BPlusNode::block(int i) {
    return (void*)((uchar*)val(i) - 6);
}
void BPlusNode::setBlock(int i, void* pbData) {
    memcpy(block(i), pbData, getAttrLen() + 6);
    owner->bpm->markDirty(pageIndex);
    //owner->bpm->writeBack(pageIndex);
}
void BPlusNode::clearBlock(int i) {
    memset(block(i), 0, getAttrLen() + 6);
    owner->bpm->markDirty(pageIndex);
    //owner->bpm->writeBack(pageIndex);
}

void* BPlusNode::val(int i) const {
    int offs = 16 + (getAttrLen() + 6) * i;
    return (void*)(page + offs);
}
void BPlusNode::setVal(int i, void* pData) {
    int offs = 16 + (getAttrLen() + 6) * i;
    memcpy(page + offs, pData, getAttrLen());
    owner->bpm->markDirty(pageIndex);
    //owner->bpm->writeBack(pageIndex);
}

int BPlusNode::child(int i) const {
    int offs = 10 + (getAttrLen() + 6) * i;
    return *((int*)(page + offs));
}
void BPlusNode::setChild(int i, int ch) {
    int offs = 10 + (getAttrLen() + 6) * i;
    *((int*)(page + offs)) = ch;
    owner->bpm->markDirty(pageIndex);
    //owner->bpm->writeBack(pageIndex);
}

int BPlusNode::lastPtr() const {
    return child(owner->fanOut);
}
void BPlusNode::setLastPtr(int p) {
    setChild(owner->fanOut, p);
    //owner->bpm->writeBack(pageIndex);
}

RID BPlusNode::rec(int i) const {
    int offs = 10 + (getAttrLen() + 6) * i;
    int* ptrPage = (int*)(page + offs);
    short* ptrSlot = (short*)(page + offs + 4);
    return RID(*ptrPage, *ptrSlot);
}
void BPlusNode::setRec(int i, const RID& rid) {
    int offs = 10 + (getAttrLen() + 6) * i;
    int* ptrPage = (int*)(page + offs);
    short* ptrSlot = (short*)(page + offs + 4);
    *ptrPage = rid.getPage();
    *ptrSlot = rid.getSlot();
    owner->bpm->markDirty(pageIndex);
    //owner->bpm->writeBack(pageIndex);
}