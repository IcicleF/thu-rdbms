#include "ix/ix_handler.h"
#include <iostream>
#include <cstring>

using namespace std;

IXHandler::IXHandler()
{

}

IXHandler::IXHandler(BPlusTree *bpt, AttrType attrtype, int attrlen)
{
    int ai,bi;
    float af,bf;
    this->bpt = bpt;
    this->attrtype = attrtype;
    this->attrlen = attrlen;
    bpt->cmp = [&] (void *a, void *b) -> int {
        if (this->attrtype == INTEGER){
            ai = *((int*)a);
            bi = *((int*)b);
            if(ai < bi) return -1;
            else if(af == bf)return 0;
            else return 1;
        }
        else if(this->attrtype == FLOAT){
            af = *((float*)a);
            bf = *((float*)b);
            if(af < bf)return -1;
            else if(af == bf)return 0;
            else return 1;
        }
        else{
            return strncmp((char*)a, (char*)b, this->attrlen);
        }
    };
}

IXHandler::~IXHandler()
{
    delete this->bpt;
}

int IXHandler::insertEntry(void* pData, const RID& rid) {
    bpt->insertEntry(pData, rid);
    return 0;
}

int IXHandler::deleteEntry(void* pData, const RID& rid) {
    return bpt->deleteEntry(pData, rid);
}