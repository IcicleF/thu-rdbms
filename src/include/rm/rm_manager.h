#ifndef RM_FILEMANAGER_H
#define RM_FILEMANAGER_H

#include "errs.h"
#include "rm/rm_file.h"
#include "pf/bufmanager/BufPageManager.h"
#include "pf/fileio/FileManager.h"
#include "pf/utils/pagedef.h"

class RMManager {
        FileManager *fm;
        BufPageManager *bpm;
    public:
        RMManager();
        RMManager(FileManager *fm, BufPageManager *bpm);
        ~RMManager();
        
        void createFile(const char*, int);
        RMFile openFile(const char*);
        void closeFile(RMFile&);
};

#endif