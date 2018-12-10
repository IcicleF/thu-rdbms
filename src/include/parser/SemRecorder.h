#ifndef PARSER_SEMRECORDER_H
#define PARSER_SEMRECORDER_H

#include <vector>
#include <string>

class SemRecorder {
    public:
        SemRecorder() { 
            inField = false;
            exprLayers = 0;
        }
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
        bool inField;
        int exprLayers;
};

#endif