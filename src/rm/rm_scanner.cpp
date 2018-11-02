#include "rm/rm_scanner.h"

RMScanner::RMScanner()
{
}

RMScanner::~RMScanner()
{
}

RMScanner::ScanPage(const RMFile &rmf, CharBufType b, int AttrLength, int AttrOffset)
{
    PI temp;
    ushort occ = rmf.getOccPtr(b);
    ushort *temp = new char[AttrLength + 5];
    while(true){
        for (int i = 0 ; i < AttrLength ; i++)temp[i] = b[occ + i];
        
    }
}

RMScanner::OpenScan(const RMFile &rmf, AttrType type, int AttrLength, int AttrOffset, ScanType comp, void *value){
{
    int PageId, index;
    CharBufType b;
    PageId = 0;
    res.clear();
    while (true) {
        CharBufType b = (CharBufType)rmf.bpm -> getPage(rmf.fileId, PageId, index);
        if (b[USED_SIZE_LOC] == 0)break;
        ScanPage(rmf, b, AttrLength, AttrOffset);
        PageId++;
    }
}