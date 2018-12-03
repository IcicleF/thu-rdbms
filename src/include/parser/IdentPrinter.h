#ifndef PARSER_IDENTPRINTER_H
#define PARSER_IDENTPRINTER_H

#include <string>

class IdentPrinter {
    public:
        IdentPrinter() : level(0) { }
        void ident();
        void deident();
        void writeln(const std::string&);

    private:
        int level;
};

#endif