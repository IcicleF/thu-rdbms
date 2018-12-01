#ifndef PARSER_IDENTPRINTER
#define PARSER_IDENTPRINTER

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