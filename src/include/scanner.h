#ifndef SCANNER_H
#define SCANNER_H

enum AttrType {
    INTEGER,
    FLOAT,
    STRING
};

enum ScanType {
    ST_EQ,
    ST_LT,
    ST_GT,
    ST_LE,
    ST_GE,
    ST_NE,
    ST_NOP
};

#endif