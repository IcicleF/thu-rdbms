#include "ix/ix_manager.h"

IXManager::IXManager(FileManager *fm, BufPageManager *bpm)
{
    this->fm = fm;
    this->bpm = bpm;
}

IXManager::~IXManager() {
    // Nothing special.
}

int IXManager::createIndex(const char *fileName, int indexNo, AttrType attrtype, int attrLength)
{
    string ss;
    int fileId,index;
    BufType b;

    ss = fileName + to_string(indexNo) + ".index";
    fm->createFile(ss.c_str());
    fm->openFile(ss.c_str(), fileId);
    b = bpm->allocPage(fileId, 0, index, false);
    b[0] = attrLength;
    b[1] = attrtype;
    b[2] = 1;//num of node
    b[3] = 1;
    //init
    bpm->markDirty(index);
    b = bpm->allocPage(fileId, 1, index, false);
    b[0] = 0x00020000;
    bpm->markDirty(index);
    bpm->writeBack(index);
    return 0;
}

IXHandler* IXManager::openIndex(const char *fileName, int indexNo)
{
    string ss;
    int len,index;
    BufType b;

    ss = fileName + to_string(indexNo) + ".index";

    if (fileIds.find(ss) != fileIds.end())
        return NULL;

    int fileId;
    if( !(fm->openFile(ss.c_str(), fileId)) ) {
        //error
        return NULL;
    }

    fileIds[ss] = fileId;

    b = bpm->getPage(fileId, 0, index);

    BPlusTree *bpt;
    bpt = new BPlusTree(this->bpm, fileId);
    IXHandler *ih = new IXHandler(bpt, (AttrType)b[1], (int)b[0]);
    ih->fileName = ss;
    return ih;
}

int IXManager::closeIndex(IXHandler &ih)
{
    if (fileIds.find(ih.fileName) != fileIds.end()) {
        fm->closeFile(fileIds[ih.fileName]);
        fileIds.erase(ih.fileName);
    }
    return 0;
}

int IXManager::destroyIndex(const char* fileName, int indexNo) {
    string fn = string(fileName) + to_string(indexNo) + ".index";
    if (fileIds.find(fn) != fileIds.end()) {
        fm->closeFile(fileIds[fn]);
        fileIds.erase(fn);
    }
    remove(fn.c_str());
}