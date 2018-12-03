#include <iostream>
#include <algorithm>
#include "parser/SemRecorder.h"

using namespace std;

void SemRecorder::beginField() {
    symStack.clear();
    inField = true;
}

void SemRecorder::endField() {
    inField = false;
}

bool SemRecorder::isInField() {
    return inField;
}

bool SemRecorder::push(const string& ident) {
    if (!inField)
        return true;
    string locase = ident;
    transform(locase.begin(), locase.end(), locase.begin(), ::tolower);
    for (const string& ex : symStack)
        if (ex == locase) {
            cout << "invalid duplicate definition: " << ident << endl;
            return false;
        }
    symStack.push_back(locase);
    return true;
}

bool SemRecorder::check(const string& ident) {
    if (!inField)
        return true;
    string locase = ident;
    transform(locase.begin(), locase.end(), locase.begin(), ::tolower);
    for (const string& ex : symStack)
        if (ex == locase)
            return true;
    cout << "undefined identifier: " << ident << endl;
    return false;
}

void SemRecorder::beginExpr() {
    inExpr = true;
}

void SemRecorder::endExpr() {
    inExpr = false;
}

bool SemRecorder::isInExpr() {
    return inExpr;
}