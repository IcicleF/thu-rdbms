#include "errs.h"

void raise(int err) {
    __ERR_STORAGE::instance()->seterr(err);
}

int errcode() {
    return __ERR_STORAGE::instance()->geterr();
}