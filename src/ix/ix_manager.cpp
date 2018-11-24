#include "ix/ix_manager.h"

IXManager::IXManager(FileManager *fm, BufPageManager *bpm)
{
    this->fm = fm;
    this->bpm = bpm;
}

int IXManager::createIndex(const char *fileName, int indexNo, AttrType attrtype, int attrLength)
{
    string ss;
    int fileId,index;
    BufType b;

    ss = fileName + to_string(indexNo);
    int len = ss.length();
    char *finalfn;
    finalfn = new char[len+2];
    for(int i = 0; i < len; i++){
        *(finalfn + i) = ss[i];
    }
    *(finalfn + len) = 0;
    fm->createFile(finalfn);
    fm->openFile(finalfn, fileId);
    b = bpm->allocPage(fileId, 0, index, false);
    {
        b[0] = attrLength;
        b[1] = attrtype;
        //init
    }
}

IXHandler* IXManager::openIndex(const char *fileName, int indexNo)
{
    string ss;
    int len,index;
    BufType b;

    ss = fileName + to_string(indexNo);
    len = ss.length();
    char *finalfn = new char[len + 2];
    for (int i = 0; i < len; i++){
        *(finalfn + i) = ss[i];
    }
    *(finalfn + len) = 0;

    int fileId;
    if( !(fm->openFile(finalfn, fileId)) ) {
        //error
        return;
    }

    b = bpm->getPage(fileId, 0, index);

    BPlusTree *bpt;
    bpt = new BPlusTree(this->bpm, fileId, b[0]);
    IXHandler *ih = new IXHandler(bpt);
    return ih;
}

int IXManager::closeIndex(IXHandler &ih)
{

}