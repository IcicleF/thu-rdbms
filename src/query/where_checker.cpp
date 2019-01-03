#include "query/where_checker.h"
#include "global.h"
#include <cassert>
#include <iostream>

using namespace std;

extern Global* global;

inline bool checkDate(int yy, int mm, int dd) {
    if (yy < 1970 || yy > 9999)
        return false;
    if (mm < 1 || mm > 12)
        return false;
    if (dd < 1)
        return false;
    int dlim;
    if ((mm < 8 && (mm & 1)) || (mm >= 8 && ((mm & 1) ^ 1)))
        dlim = 31;
    else if (mm != 2)
        dlim = 30;
    else {
        dlim = 28;
        if (yy % 4 == 0)
            dlim = 29;
        if (yy % 100 == 0)
            dlim = 28;
        if (yy % 400 == 0)
            dlim = 29;
    }
    return dd <= dlim;
}

bool checkDateStr(string strval) {
    if (strval.length() != 10)
        return false;
    for (int i = 0; i < 10; i++){
        if (i == 4 || i == 7) {
            if (strval[i] != '-' && strval[i] != '/')
                return false;
        }
        else{
            if (strval[i] > '9' || strval[i] < '0')
                return false;
        }
    }
    int year = atoi(strval.substr(0,4).c_str());
    int month = atoi(strval.substr(5,2).c_str());
    int day = atoi(strval.substr(8,2).c_str());
    return checkDate(year, month, day);
}

ExprType getColumn(const RMRecord& rec, string tableName, string colName) {
    ColInfo* cInfo = NULL;
    try {
        cInfo = global->ql->db_info->TableMap[tableName]->ColMap[colName];
    }
    catch (exception e) {
        throw EvalException("unknown table or column name");
    }

    ExprType result;
    int offset = cInfo->AttrOffset;
    int len = cInfo->AttrLength;
    char* data = new char[8192];
    rec.getData(data);
    switch (cInfo->type) {
        case INTEGER:
            assert(len == 4);
            result.type = TYPE_INT;
            result.val = *((int*)(data + offset));
            break;
        case FLOAT:
            assert(len == 4);
            result.type = TYPE_FLOAT;
            result.floatval = *((float*)(data + offset));
            break;
        case STRING:
            result.type = cInfo->asttype;
            result.strval = new char[len + 5];
            strncpy(result.strval, data + offset, len);
            result.strval[len] = 0;
            break;
    }
    delete[] data;
    return result;
}

inline bool isNull(AstBase* ast) {
    return ast->type == AST_KEYWORD && ast->val == AST_NULL;
}

inline ExprType fetchValue(AstCol* col, const map<string, RMRecord>& recs) {
    if (col->owner == NULL) {
        if (recs.size() != 1)
            throw EvalException("must specify table name of column: " + dynamic_cast<AstIdentifier*>(col->colName)->toString());
        else 
            for (auto kv : recs)
                return getColumn(kv.second, kv.first, dynamic_cast<AstIdentifier*>(col->colName)->toString());
    }
    else {
        auto tableName = dynamic_cast<AstIdentifier*>(col->owner)->toString();
        if (recs.find(tableName) == recs.end())
            throw EvalException("unknown table: " + tableName);
        return getColumn(recs.at(tableName), tableName, dynamic_cast<AstIdentifier*>(col->colName)->toString());
    }
    return ExprType();
}

bool checkWhere(AstBase* _wh, const map<string, RMRecord>& recs) {
    if (_wh == NULL)
        return true;
    auto wh = dynamic_cast<AstWhereClause*>(_wh);
    switch (wh->val) {
        case WHERE_AND:
            return checkWhere(wh->lhs, recs) && checkWhere(wh->rhs, recs);
        case WHERE_OR:
            return checkWhere(wh->lhs, recs) || checkWhere(wh->rhs, recs);
        case WHERE_NOT:
            return !checkWhere(wh->lhs, recs);
        default:
            {
                ExprType vl = fetchValue(dynamic_cast<AstCol*>(wh->lhs), recs);
                bool is_null = false;
                if (vl.type == TYPE_INT)
                    is_null = (vl.val == -1);
                else if (vl.type == TYPE_FLOAT)
                    is_null = ((int)vl.floatval == -1);
                else
                    is_null = (vl.strval[0] == (char)(-1));
                if (wh->val == WHERE_IS_NOT_NULL || wh->val == WHERE_IS_NULL)
                    return (wh->val == WHERE_IS_NULL) == is_null;
            
                if (isNull(wh->rhs) || is_null)
                    return false;
                ExprType vr;
                if (wh->rhs->type == AST_COL)
                    vr = fetchValue(dynamic_cast<AstCol*>(wh->rhs), recs);
                else
                    vr = calcExpr(wh->rhs);
                
                bool vlStr = (vl.type != TYPE_INT) && (vl.type != TYPE_FLOAT);
                bool vrStr = (vr.type != TYPE_INT) && (vr.type != TYPE_FLOAT);
                bool compat = vlStr == vrStr;
                if (compat && vl.type == TYPE_DATE) {
                    if (wh->rhs->type == AST_COL)
                        compat = vr.type == TYPE_DATE;
                    else
                        compat = checkDateStr(vr.strval);
                }
                
                float diff = 0;
                if (!compat)
                    throw EvalException("comparing incompatible types");
                else if (vlStr)
                    diff = (float)strcmp(vl.strval, vr.strval);
                else
                    diff = (float)(vl.type == TYPE_INT ? vl.val : vl.floatval) - (vr.type == TYPE_INT ? vr.val : vr.floatval);
                
                const float EPS = 1e-6;
                switch (wh->val) {
                    case WHERE_EQ:
                        return abs(diff) < EPS;
                    case WHERE_NE:
                        return abs(diff) > EPS;
                    case WHERE_LT:
                        return diff < -EPS;
                    case WHERE_GT:
                        return diff > EPS;
                    case WHERE_LE:
                        return diff < EPS;
                    case WHERE_GE:
                        return diff > -EPS;
                    default:
                        throw EvalException("unknown logic operator type");
                }
            }
    }
    return false;
}

ExprType calcExpr(AstBase* expr) {
    // expr cannot be instance of AstNull because we have checked it in checkWhere
    ExprType res;
    if (expr->type == AST_LITERAL) {
        res.type = dynamic_cast<AstLiteral*>(expr)->literalType;
        if (res.type == TYPE_INT)
            res.val = expr->val;
        else if (res.type == TYPE_FLOAT)
            res.floatval = expr->floatval;
        else
            res.strval = expr->strval;          // do not copy string here because expr remains unchanged (till now)
    }
    else {
        auto e = dynamic_cast<AstExpr*>(expr);
        ExprType lhs = calcExpr(e->lhs);
        assert(lhs.type == TYPE_INT || lhs.type == TYPE_FLOAT);
        if (e->val != EXPR_UMINUS) {
            ExprType rhs = calcExpr(e->rhs);
            assert(rhs.type == TYPE_INT || rhs.type == TYPE_FLOAT);

            if (lhs.type == TYPE_INT && rhs.type == TYPE_INT) {
                res.type = TYPE_INT;
                res.val = (e->val == EXPR_ADD ? lhs.val + rhs.val : lhs.val - rhs.val);
            }
            else {
                res.type = TYPE_FLOAT;
                float lval = (lhs.type == TYPE_INT) ? (float)(lhs.val) : lhs.floatval;
                float rval = (rhs.type == TYPE_INT) ? (float)(rhs.val) : rhs.floatval;
                res.floatval = (e->val == EXPR_ADD ? lval + rval : lval - rval);
            }
        }
        else {
            res.type = lhs.type;
            if (lhs.type == TYPE_INT)
                res.val = -lhs.val;
            else
                res.floatval = -lhs.floatval;
        }
    }
    return res;
}