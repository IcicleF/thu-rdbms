#ifndef ERRS_H
#define ERRS_H

#include <cstddef>

const int E_OK = 0;
const int E_RM_FAILURE = 101;
const int E_RM_NOBUFMGR = 102;
const int E_RM_INVSLOT = 103;
const int E_RM_INVEPTR = 104;
const int E_RM_TLRCSIZE = -1;
const int E_RM_WRONGSCANTYPE = -2;

class __ERR_STORAGE {
    public:
        static __ERR_STORAGE* instance() {
            static __ERR_STORAGE* inst;
            if (inst == NULL)
                inst = new __ERR_STORAGE;
            return inst;
        }
        void seterr(int __errc) {
            errc = __errc;
        }
        int geterr() {
            return errc;
        }
    private:
        __ERR_STORAGE() { }
        int errc;
};

void raise(int err);
int errcode();

#endif