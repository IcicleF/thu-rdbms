// This is currently just an example.

#include "pf/bufmanager/BufPageManager.h"
#include "pf/fileio/FileManager.h"
#include "rm/rm_manager.h"
#include "rm/rm_scanner.h"
#include "ix/ix_manager.h"
#include "ix/ix_handler.h"
#include <iostream>
#include <cstring>

using namespace std;

void test1(FileManager* fm, BufPageManager* bpm) {
    char dat[25];

    RMManager manager(fm, bpm);
    manager.createFile("test.txt", 20);
    RMFile fh = manager.openFile("test.txt");

    RID r1 = fh.insertRec("aaaaabbbbbcccccddddd");
    RID r2 = fh.insertRec("eeeeefffffggggghhhhh");
    RID r3 = fh.insertRec("aaaaabbbbbcccccddddd");
    RID r4 = fh.insertRec("eeeeefffffggggghhhhh");

    fh.deleteRec(r3);

    RID r5 = fh.insertRec("abcde1234567890fghij");

    RMRecord rc1 = fh.getRec(r1);
    rc1.getData(dat);
    cout << dat << endl;

    rc1 = fh.getRec(r2);
    rc1.getData(dat);
    cout << dat << endl;

    rc1 = fh.getRec(r5);
    rc1.getData(dat);
    cout << dat << endl;

    cout << "Err=" << errcode() << endl;

    RMScanner sc;
    char *val = new char[6];
    strcpy(val, "ddddd");
    RMRecord rec;

    sc.nextRec(rec);
    sc.openScan(fh, STRING, 5, 10, ST_LE, (void *)val);
    while (sc.nextRec(rec)) {
        rec.getData(dat);
        cout << "Dat="<< dat << endl;
        //cin >> dat;
    }
    cout << "fin" << endl;
}

void test2(FileManager* fm, BufPageManager* bpm) {
    IXManager ixmgr(fm, bpm);
    ixmgr.createIndex("temp", 0, STRING, 4);
    IXHandler* ih = ixmgr.openIndex("temp", 0);

    cout << "success: build ih instance" << endl;

    auto inse = [&](const char* s, int page, short slot) -> void {
        ih->insertEntry((void*)s, RID(page, slot));
    };
    inse("abcd", 10, 343);
    cout << "success: 1" << endl;

    inse("01z", 4, 239);
    cout << "success: 2" << endl;

    inse("0", 122349, 16);
    cout << "success: 3" << endl;
    
    inse("2122", 212, 44);
    cout << "success: 4" << endl;

    inse("abzz", 2, 1349);
    cout << "success: 5" << endl;

    inse("wsgi", 23, 16);
    cout << "success: 6" << endl;

    //ih->bpt->printTree();
    cout << "Insert Test succeed ! " << endl;

    auto dele = [&](const char* s, int page, short slot) -> void {
        ih->deleteEntry((void *)s, RID(page, slot));
    };
    dele("2122", 212, 44);
    ih->bpt->printTree();
}

int main() {
    FileManager *fm = new FileManager();
    BufPageManager *bpm = new BufPageManager(fm);

    // test1(fm, bpm);
    test2(fm, bpm);
    
    return 0;
}