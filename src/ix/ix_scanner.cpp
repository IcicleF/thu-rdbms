#include "ix/ix_scanner.h"

IXScanner::IXScanner(){
}

IXScanner::~IXScanner(){
}

void IXScanner::openScan(IXHandler &ih, ScanType scantype, void *pData)
{
    this->scanstatus = 1;
    this->scantype = scantype;
    this->bpt = ih.bpt;
    cur.owner = this->bpt;
    cur.pageId = this->bpt->root();
    cur.getPage();
    while(cur.type() != BT_LEAF){
        cur.pageId = cur.child(0);
        cur.getPage();
    }


}