#ifndef ERRS_H
#define ERRS_H

const int E_OK = 0;
const int E_RM_FAILURE = 101;
const int E_RM_NOTFOUND = 102;

namespace __ERR_STORAGE {
    int __errcode;
}

void raise(int err) {
    __ERR_STORAGE::__errcode = err;
}

int errcode() {
    return __ERR_STORAGE::__errcode;
}

#endif