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
#include "query/ql_manager.h"

struct Global {
    std::vector<std::string> strList;

    MetaManager* mm;
    FileManager* fm;
    BufPageManager* bpm;
    RMManager *rm;
    IXManager *ix;
    QLManager *ql;

    Global() {
        fm = new FileManager();
        bpm = new BufPageManager(fm);
        rm = new RMManager(fm, bpm);
        ix = new IXManager(fm, bpm);
        ql = new QLManager(rm, ix);
        mm = new MetaManager(rm, ix, ql);
    }

    ~Global() {
        delete rm;
        delete ix;
        delete bpm;
        delete fm;
        delete mm;
    }
};

#endif