#ifndef PARSER_SEMRECORDER_H
#define PARSER_SEMRECORDER_H

#include <vector>
#include <string>

class SemRecorder {
    public:
        SemRecorder() { }
        ~SemRecorder() { }

        // AstField
        void beginField();
        void endField();
        bool isInField();
        bool push(const std::string&);
        bool check(const std::string&);

        // AstExpr
        void beginExpr();
        void endExpr();
        bool isInExpr();

    private:
        std::vector<std::string> symStack;
        bool inField, inExpr;
};

#endif