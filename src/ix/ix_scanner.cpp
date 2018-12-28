#include "ix/ix_scanner.h"

IXScanner::IXScanner(){
}

IXScanner::~IXScanner(){
}

void IXScanner::openScan(IXHandler &ih, ScanType scantype, void *pData)
{
    scanstatus = 1;
    this->scantype = scantype;
    this->attrlen = ih.attrlen;
    bpt = ih.bpt;
    standard = pData;
    cur.owner = bpt;
    stdcur.owner = bpt;
    cur.pageId = bpt->root();
    cur.getPage();
    while(cur.type() != BT_LEAF){
        cur.pageId = cur.child(0);
        cur.getPage();
    }
    curchild = 0;
    if(scantype != ST_NOP && scantype != ST_NE){
        bpt->traceToLeaf(pData);
        stdcur.pageId = bpt->cur.pageId;
        stdcur.getPage();
        stdpos = -1;
        for (int i = 0; i < stdcur.count(); i++){
            if( bpt->cmp(pData, stdcur.val(i)) == 0 ){
                stdpos = i;
                break;
            }
        }
    }
    if(scantype == ST_EQ || scantype == ST_GE || scantype == ST_GT){
        cur.pageId = stdcur.pageId;
        if (stdpos == -1) curchild = 0;
        else curchild = stdpos;
        cur.getPage();
    }
    pf = true;
}

void IXScanner::getnextRec()
{
    if(curchild == cur.count()){
        curchild = 0;
        cur.pageId = cur.child(cur.count());
        cur.getPage();
    }
    else curchild++;
}

bool IXScanner::nextRec(RID &rid, void* inx)
{
    if (this->scanstatus == 0)return false;
    switch(scantype)
    {
        ST_EQ:
        {
            if(stdpos == -1)return false;
            else{
                if(pf){
                    rid = stdcur.rec(stdpos);
                    memcpy(inx, stdcur.val(stdpos), attrlen);
                    pf = false;
                    return true;
                }
                else return false;
            }
        }
        ST_GE:
        {
            while(true){
                if(cur.pageId == 0)return false;
                if(bpt->cmp(cur.val(curchild), standard) >= 0){
                    rid = cur.rec(curchild);
                    memcpy(inx, cur.val(curchild), attrlen);
                    getnextRec();
                    return true;
                }
                getnextRec();
            }
        }
        ST_GT:
        {
            while(true){
                if(cur.pageId == 0)return false;
                if(bpt->cmp(cur.val(curchild), standard) > 0){
                    rid = cur.rec(curchild);
                    memcpy(inx, cur.val(curchild), attrlen);
                    getnextRec();
                    return true;
                }
                getnextRec();
            }
        }
        ST_LE:
        {
            if (cur.pageId == 0)return false;
            if (bpt->cmp(cur.val(curchild), standard) > 0)pf = false;
            if (pf == false)return false;
            rid = cur.rec(curchild);
            memcpy(inx, cur.val(curchild), attrlen);
            getnextRec();
            return true;
        }
        ST_LT:
        {
            if (cur.pageId == 0)return false;
            if (bpt->cmp(cur.val(curchild), standard) >= 0)pf = false;
            if (pf == false)return false;
            rid = cur.rec(curchild);
            memcpy(inx, cur.val(curchild), attrlen);
            getnextRec();
            return true;
        }
        ST_NE:
        {
            while(true){
                if (cur.pageId == 0)return false;
                if (bpt->cmp(cur.val(curchild), standard) != 0){
                    rid = cur.rec(curchild);
                    memcpy(inx, cur.val(curchild), attrlen);
                    getnextRec();
                    return true;
                }
                getnextRec();
            }
        }
        ST_NOP:
        {
            while(true){
                if(cur.pageId == 0)return false;
                rid = cur.rec(curchild);
                memcpy(inx, cur.val(curchild), attrlen);
                getnextRec();
                return true;
            }
        }
        default:
        {
            return false;
        }
    }

    return true;
}

void IXScanner::closeScan()
{
    this->scanstatus = 0;
}