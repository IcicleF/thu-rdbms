#include "rm/rm_manager.h"

RMManager::RMManager()
{
    fm = new FileManager();
    bpm = new BufPageManager(fm);
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
    }
    else{
        fm -> createFile(fileName);
        fm -> openFile(fileName, fileId);
        b = bpm -> allocPage(fileId, 0, index, false);
        b[0] = recordSize;
        bpm -> markDirty(index);
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