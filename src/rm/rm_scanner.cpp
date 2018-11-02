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
    int vi,stdi;
    float vf,stdf;
    string vs,stds;
    
    if(value==NULL)return true;
    switch(type)
    {
        case INTEGER:
        {
            vi = atoi((char *)b);
            stdi = atoi((char *)value);
            break;
        }
        
        case FLOAT:
        {
            vf = atof((char *)b);
            stdf = atof((char *)value);
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

void RMScanner::ScanPage(const RMFile &rmf, CharBufType b, AttrType type, int AttrLength, int AttrOffset, ScanType comp, void *value)
{
    ushort occ = rmf.getOccPtr(b);
    CharBufType temp = new uchar[AttrLength + 5];
    CharBufType tb;
    ushort* tnext = new ushort;
    RID trid;
    RMRecord trec;
    
    tb = b + occ;
    (*tnext) = occ;
    int recSize = rmf.recSize;
    while (true){
        for (int i = 0 ; i < AttrLength ; i++)temp[i] = tb[AttrOffset + i];
        temp[AttrLength] = 0;//cut out the value from the record
        if(vcompare(type, comp, temp, value)){
            trid = RID(PageId, (*tnext));
            trec = RMRecord(trid, (char *)tb);
            res.push_back(trec);
        }
        tnext = NEXT(tb);
        if( (*tnext)==0 ) break;
        tb = b + (*tnext);
    }
    delete temp;
}

//Scan all the records that satisfies the scan condition
void RMScanner::openScan(const RMFile &rmf, AttrType type, int AttrLength, int AttrOffset, ScanType comp, void *value){
    int index;
    CharBufType b;
    PageId = 0;
    res.clear();
    limit = 0;
    bz = 0;
    while (true) {
        CharBufType b = (CharBufType)rmf.bpmgr -> getPage(rmf.fileId, PageId, index);
        if (b[USED_SIZE_LOC] == 0)break;
        ScanPage(rmf, b, type, AttrLength, AttrOffset, comp, value);
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

void RMScanner::closeScan()
{
    res.clear();
    limit = 0;
    bz = 0;
}