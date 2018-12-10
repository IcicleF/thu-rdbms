#ifndef GLOBAL_H
#define GLOBAL_H

#include <vector>
#include <string>

#include "pf/bufmanager/BufPageManager.h"
#include "pf/fileio/FileManager.h"
#include "rm/rm_manager.h"
#include "rm/rm_scanner.h"
#include "ix/ix_manager.h"
#include "ix/ix_handler.h"
#include "meta/MetaManager.h"

struct Global {
    std::vector<std::string> strList;

    MetaManager* mm;
    FileManager* fm;
    BufPageManager* bpm;

    Global() {
        mm = new MetaManager();
        fm = new FileManager();
        bpm = new BufPageManager(fm);
    }

    ~Global() {
        delete bpm;
        delete fm;
        delete mm;
    }
};

#endif