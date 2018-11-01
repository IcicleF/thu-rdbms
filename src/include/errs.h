#ifndef ERRS_H
#define ERRS_H

const int E_OK = 0;
const int E_RM_FAILURE = 101;
const int E_RM_NOBUFMGR = 102;
const int E_RM_INVSLOT = 103;
const int E_RM_INVEPTR = 104;

namespace __ERR_STORAGE {
    int __errcode = E_OK;
}

void raise(int err) {
    __ERR_STORAGE::__errcode = err;
}

int errcode() {
    return __ERR_STORAGE::__errcode;
}

#endif