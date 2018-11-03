// This is currently just an example.

#include "rm/rm_manager.h"
#include "rm/rm_scanner.h"
#include <iostream>
#include <cstring>

using namespace std;

int main() {
    char dat[25];

    RMManager manager;
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
    return 0;
}