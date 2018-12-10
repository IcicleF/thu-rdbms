// This is currently just an example.


#include "parser.tab.hpp"
#include "global.h"

using namespace std;

Global* global;

/*
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

    inse("0\0\0\0", 122349, 16);
    cout << "success: 3" << endl;
    
    inse("2122", 212, 44);
    cout << "success: 4" << endl;

    inse("abzz", 2, 1349);
    cout << "success: 5" << endl;

    inse("wsgi", 23, 16);
    cout << "success: 6" << endl;

    inse("f*0k", 120, 10);
    inse("f*1k", 121, 10);
    inse("f*2k", 122, 10);
    inse("f*3k", 123, 10);
    inse("f*4k", 124, 10);
    inse("f*5k", 125, 10);

    inse("f*6k", 126, 10);
    inse("f*7k", 127, 10);
    inse("f*8k", 128, 10);
    cout << "success: f*0k ~ f*8k" << endl;

    ih->bpt->printTree();
    cout << "Insert Test succeed ! " << endl;

    auto dele = [&](const char* s, int page, short slot) -> void {
        ih->deleteEntry((void *)s, RID(page, slot));
    };
    dele("2122", 212, 44);
    dele("abcd", 10, 343);
    dele("01z", 4, 239);
    dele("abzz", 2, 1349);
    dele("0", 122349, 16);
    ih->bpt->printTree();
}
*/

int main() {
    global = new Global();
    yyparse();

    return 0;
}