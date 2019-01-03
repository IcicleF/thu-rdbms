#include "rm/rm_scanner.h"
#include "errs.h"
#include<cstdlib>
#include<cstdio>

using namespace std;

int PageId;

RMScanner::RMScanner()
{
    Length = -1;
    Offset = -1;
}

RMScanner::~RMScanner()
{
}

bool RMScanner::vcompare(CharBufType b)
{
    int vi,stdi;
    float vf,stdf;
    string vs,stds;
    
    if(value==NULL)return true;
    switch(type)
    {
        case INTEGER:
        {
            vi = *((int*)b);
            stdi = *((int*)value);
            break;
        }
        
        case FLOAT:
        {
            vf = *((float*)b);
            stdf = *((float*)value);
            break;
        }
        
        case STRING:
        {
            vs = (char *)b;
            stds = (char *)value;
            break;
        }
        
        default:
        {
            raise(E_RM_WRONGSCANTYPE);
            return false;
        }
    }
    
    switch(comp)
    {
        case ST_EQ:
        {
            if(type==INTEGER)return vi==stdi;
            if(type==FLOAT)return vf==stdf;
            if(type==STRING)return vs==stds;
        }
        case ST_LT:
        {
            if(type==INTEGER)return vi<stdi;
            if(type==FLOAT)return vf<stdf;
            if(type==STRING)return vs<stds;
        }
        case ST_GT:
        {
            if(type==INTEGER)return vi>stdi;
            if(type==FLOAT)return vf>stdf;
            if(type==STRING)return vs>stds;
        }
        case ST_LE:
        {
            if(type==INTEGER)return vi<=stdi;
            if(type==FLOAT)return vf<=stdf;
            if(type==STRING)return vs<=stds;
        }
        case ST_GE:
        {
            if(type==INTEGER)return vi>=stdi;
            if(type==FLOAT)return vf>=stdf;
            if(type==STRING)return vs>=stds;
        }
        case ST_NE:
        {
            if(type==INTEGER)return vi!=stdi;
            if(type==FLOAT)return vf!=stdf;
            if(type==STRING)return vs!=stds;
        }
        case ST_NOP:
        {
            return true;
        }
    }
    return false;
}

void RMScanner::GetNextRec()
{
    CharBufType tb;
    int recSize = rmf->recSize;
    int index;
    ushort *tnext;
    tb = curb + curslotId;
    tnext = NEXT(tb);
    if ((*tnext)==0){
        curpageId++;
        curb = (CharBufType)rmf->bpmgr->getPage(rmf->fileId, curpageId, index);
        if( curb[USED_SIZE_LOC] == 0) curslotId = 0;
        else curslotId = rmf->getOccPtr(curb);
    }
    else curslotId = (*tnext);
}

//Scan all the records that satisfies the scan condition
void RMScanner::openScan(RMFile &rmf, AttrType type, int AttrLength, int AttrOffset, ScanType comp, void *value){
    int index;
    this->rmf = &rmf;
    curpageId = 0;
    curb = (CharBufType) rmf.bpmgr -> getPage(rmf.fileId, curpageId, index);
    if (curb[USED_SIZE_LOC] == 0)curslotId = 0;
    else curslotId = rmf.getOccPtr(curb);
    Length = AttrLength;
    Offset = AttrOffset;
    this->type = type;
    this->comp = comp;
    this->value = value;
}

bool RMScanner::nextRec(RMRecord &rc)
{
    CharBufType temp = new uchar[Length + 5];
    CharBufType tb;
    RID trid;

    if (Length == -1 && Offset == -1)return false;
    if (curb[USED_SIZE_LOC] == 0)return false;
    while (true) {
        tb = curb + curslotId;
        for(int i=0; i<Length; i++){
            temp[i] = tb[Offset + i];
        }
        temp[Length] = 0;
        if(vcompare(temp)==true){
            trid = RID(curpageId, curslotId);
            rc = rmf->getRec(trid);
            GetNextRec();
            return true;
        }
        GetNextRec();
        if (curb[USED_SIZE_LOC] == 0)return false;
    }
}

void RMScanner::closeScan()
{
    Length = -1;
    Offset = -1;
}