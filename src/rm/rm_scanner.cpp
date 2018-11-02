#include "rm/rm_scanner.h"
#include "errs.h"
#include<cstdlib>
#include<cstdio>

int PageId;

RMScanner::RMScanner()
{
}

RMScanner::~RMScanner()
{
}

bool RMScanner::vcompare(AttrType type, ScanType comp, CharBufType b, void *value)
{
    int vt,stdi;
    float vf,stdf;
    string vs,stds;
    
    if(value==NULL)return true;
    switch(type)
    {
        case INT:
        {
            vi = atoi((char *)b);
            stdi = atoi((char *)b);
            break;
        }
        
        case FLOAT:
        {
            vf = atof((char *b));
            stdf = atof((char *b));
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
            raise(WRONG_SCAN_TYPE);
            return false;
        }
    }
    
    switch(comp)
    {
        case ST_EQ:
        {
            if(type==INT)return vi==stdi;
            if(type==FLOAT)return vf==stdf;
            if(type==STRING)return vs==stds;
        }
        case ST_LT:
        {
            if(type==INT)return vi<stdi;
            if(type==FLOAT)return vf<stdf;
            if(type==STRING)return vs<stds;
        }
        case ST_GT:
        {
            if(type==INT)return vi>stdi;
            if(type==FLOAT)return vf>stdf;
            if(type==STRING)return vs>stds;
        }
        case ST_LE:
        {
            if(type==INT)return vi<=stdi;
            if(type==FLOAT)return vf<=stdf;
            if(type==STRING)return vs<=stds;
        }
        case ST_GE:
        {
            if(type==INT)return vi>=stdi;
            if(type==FLOAT)return vf>=stdf;
            if(type==STRING)return vs>=stds;
        }
        case ST_NE:
        {
            if(type==INT)return vi!=stdi;
            if(type==FLOAT)return vf!=stdf;
            if(type==STRING)return vs!=stds;
        }
        case ST_NOP:
        {
            return true;
        }
    }
}

void RMScanner::ScanPage(const RMFile &rmf, CharBufType b, int AttrLength, int AttrOffset, ScanType comp, void *value)
{
    ushort occ = rmf.getOccPtr(b);
    CharBufType temp = new uchar[AttrLength + 5];
    CharBufType tb, tnext;
    RID trid;
    RMRecord trec;
    
    tb = b + occ;
    tnext = new uchar();
    (*tnext) = occ;
    while(true){
        for (int i = 0 ; i < AttrLength ; i++)temp[i] = tb[AttrOffset + i];
        temp[AttrLength] = 0;//cut out the value from the record
        if(vcompare(type, comp, temp, value)){
            trid = RID(PageId, (*tnext));
            trec = RMRecord(trid, tb);
            res.push_back(trec);
        }
        tnext = NEXT(tb);
        if( (*tnext)==0 ) break;
        tb = b + (*tnext);
    }
    delete temp;
}

//Scan all the records that satisfies the scan condition
void RMScanner::OpenScan(const RMFile &rmf, AttrType type, int AttrLength, int AttrOffset, ScanType comp, void *value){
{
    int index;
    CharBufType b;
    PageId = 0;
    res.clear();
    limit = 0;
    bz = 0;
    while (true) {
        CharBufType b = (CharBufType)rmf.bpm -> getPage(rmf.fileId, PageId, index);
        if (b[USED_SIZE_LOC] == 0)break;
        ScanPage(rmf, b, AttrLength, AttrOffset);
        PageId++;
    }
    limit = res.size();
    bz = 0;
}

bool RMScanner::nextRec(RMRecord &rc)
{
    if(bz>=limit)return false;
    rc = res[bz];
    return true;
}

void RMScanner::CloseScan()
{
    res.clear();
    limit = 0;
    bz = 0;
}