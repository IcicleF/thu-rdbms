#include "meta/ql_manager.h"

QLManager::QLManager()
{
    //
}

QLManager::QLManager(RMManager *rm, IXManager *ix)
{
    this->rm = rm;
    this->ix = ix;
    this->db_info = NULL;
}