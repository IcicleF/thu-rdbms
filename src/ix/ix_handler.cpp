#include "ix/ix_handler.h"

IXHandler::IXHandler()
{

}

IXHandler::IXHandler(BPlusTree *bpt, AttrType attrtype)
{
    int ai,bi;
    float af,bf;
    string as,bs;

    this->bpt = bpt;
    this->attrtype = attrtype;
    auto z = [&] (void *a, void *b) -> bool {
        if (attrtype == INTEGER){
            ai = atoi((char *)a);
            bi = atoi((char *)b);
            if(ai < bi)return -1;
            else if(ai == bi)return 0;
            else return 1;
        }
        else if(attrtype == FLOAT){
            af = atof((char *)a);
            bf = atof((char *)b);
            if(af < bf)return -1;
            else if(af == bf)return 0;
            else return 1;
        }
        else{
            as = (char *)a;
            bs = (char *)b;
            if(as < bs)return -1;
            else if(as == bs)return 0;
            else return 1;
        }
    }
    bpt->cmp = z;
}

IXHandler::~IXHandler()
{
    delete this->bpt;
}