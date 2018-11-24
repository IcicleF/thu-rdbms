#include "ix/ix_handler.h"

IXHandler::IXHandler()
{

}

IXHandler::IXHandler(BPlusTree *bpt)
{
    this->bpt = bpt;
}

IXHandler::~IXHandler()
{
    delete this->bpt;
}