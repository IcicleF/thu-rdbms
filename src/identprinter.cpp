#include <iostream>
#include "identprinter.h"

using namespace std;

void IdentPrinter::ident() {
    ++level;
}

void IdentPrinter::deident() {
    if (level > 0)
        --level;
}

void IdentPrinter::writeln(const string& content) {
    for (int i = 0; i < level; ++i)
        cout << "    ";
    cout << content << endl;
}