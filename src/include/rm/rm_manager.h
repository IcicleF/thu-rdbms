#ifndef RM_FILEMANAGER_H
#define RM_FILEMANAGER_H

#include "errs.h"
#include "rm/rm_file.h"

class RMManager {
    public:
        RMManager();
        ~RMManager();
        
        void createFile(const char*, int);
        void destroyFile(const char*);
        RMFile openFile(const char*);
        void closeFile(RMFile&);
};

#endif