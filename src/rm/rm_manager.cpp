#include "rm/rm_manager.h"
#include "errs.h"

RMManager::RMManager()
{
    fm = new FileManager();
    bpm = new BufPageManager(fm);
}

RMManager::RMManager(FileManager *fm, BufPageManager *bpm)
{
    this->fm = fm;
    this->bpm = bpm;
}

RMManager::~RMManager()
{
    bpm -> close();
}

/*
    createfile and write the recordSize in Page 0
*/
void RMManager::createFile(const char *fileName, int recordSize)
{
    int fileId,index;
    BufType b;
    if (recordSize > PAGE_SIZE - PAGE_HEADER - 2 * sizeof(ushort)){
        raise(E_RM_TLRCSIZE);
    }
    else{
        fm -> createFile(fileName);
        fm -> openFile(fileName, fileId);
        b = bpm -> allocPage(fileId, 0, index, false);
        b[0] = recordSize;
        b[3] = 1;
        bpm -> markDirty(index);
        bpm->writeBack(index);
    }
}

RMFile RMManager::openFile(const char *fileName)
{
    int fileId;
    fm -> openFile(fileName, fileId);
    return RMFile(bpm, fileId);
}

void RMManager::closeFile(RMFile &RM)
{
    fm -> closeFile(RM.fileId);
}