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
    if (recordSize > PAGE_SIZE){
    }
    else{
        fm -> createfile(fileName);
        fm -> openfile(fileName, fileId);
        b = bpm -> allocPage(fileId, 0, index, false);
        b[0] = recordSize;
        bpm -> markDirty(index);
    }
}

RMFile RMManager::openFile(const char *fileName)
{
    int fileId;
    fm -> openfile(fileName, fileId);
    RMFile rs = new RMFile();
    rs -> fileId = fileId;
    return rs;
}

void RMManager::closeFile(RMFile &RM)
{
    fm -> closeFile(RM->fileId);
}