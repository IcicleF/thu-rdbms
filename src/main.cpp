// This is currently just an example.

#include "rm/rm_manager.h"
#include "rm/rm_scanner.h"
#include <iostream>
#include <cstring>

using namespace std;

int main() {
    RMManager manager;
    manager.createFile("test.txt", 20);
    RMFile fh = manager.openFile("test.txt");

    RID r1 = fh.insertRec("aaaaabbbbbcccccddddd");
    RID r2 = fh.insertRec("eeeeefffffggggghhhhh");
    RID r3 = fh.insertRec("aaaaabbbbbcccccddddd");
    RID r4 = fh.insertRec("eeeeefffffggggghhhhh");

    fh.deleteRec(r3);

    RMScanner sc;
    char *val = new char[6];
    strcpy(val, "ddddd");
    sc.openScan(fh, STRING, 5, 10, ST_LE, (void *)val);

    RMRecord rec;
    while (sc.nextRec(rec)) {
        char dat[25];
        rec.getData(dat);
        cout << "Dat="<< dat << endl;
        cin >> dat;
    }

    cout << "fin" << endl;
    return 0;
}