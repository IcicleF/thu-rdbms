// This is currently just an example.


#include "parser.tab.hpp"
#include "global.h"
#include "meta/info.h"
#include <iostream>
#include <cstdio>
#include <iterator>

using namespace std;

Global* global;

void test2() {
    FileManager* fm = new FileManager();
    BufPageManager* bpm = new BufPageManager(fm);

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
    ih->bpt->printTree();
    
    bpm->close();
    ixmgr.closeIndex(*ih);
}

/*
void testInfo()
{
    cout << global->mm->dbnum << endl;
    DBInfo* db = global->mm->dbMap["orderdb"];
    cout << db->tablenum << endl;
    TableInfo *tb;
    ColInfo *cl;
    map<string, TableInfo*>::iterator it;
    map<string, ColInfo*>::iterator iv;
    for (it = db->TableMap.begin(); it != db->TableMap.end(); it++){
        tb = it->second;
        cout << tb->name << " " << tb->colnum << " " << tb->recSize << endl;
        for (iv = tb->ColMap.begin(); iv != tb->ColMap.end(); iv++){
            cl = iv->second;
            cout << cl->name << " " << cl->AttrOffset << " " << cl->AttrLength << " ";
            cout << cl->isforeign << " " << cl->isprimary << endl;
        }
        cout << endl;
    }
}
*/
int main() {
    global = new Global();
    while (true) {
        cout << "> ";
        yyparse();
        global->bpm->close();
        cout << endl;
    }
    delete global;
    //test2();
    return 0;
}