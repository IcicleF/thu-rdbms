#ifndef GLOBAL_H
#define GLOBAL_H

#include <vector>
#include <map>
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
    std::map<std::string, std::string> settings;

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
        delete mm;
        delete ql;
        delete ix;
        delete rm;
        bpm->close();
        delete bpm;
        fm->shutdown();
        delete fm;
    }
};

#endif