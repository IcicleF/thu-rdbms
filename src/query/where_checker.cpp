#include "query/where_checker.h"
#include <cassert>
#include <iostream>

using namespace std;

extern Global* global;

ExprType getColumn(const RMRecord& rec, string tableName, string colName) {
    ColInfo* cInfo = NULL;
    try {
        global->ql->db_info->TableMap[tableName]->ColMap[colName];
    }
    catch (exception e) {
        cout << e.what() << endl;
        return ExprType();
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
            result.type = TYPE_CHAR;
            result.strval = new char[len + 5];
            strncpy(result.strval, data + offset, len);
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
}

bool checkWhere(AstBase* _wh, const map<string, RMRecord>& recs) {
    ExprType vl, vr;
    auto wh = dynamic_cast<AstWhereClause*>(_wh);
    switch (wh->val) {
        case WHERE_AND:
            return checkWhere(wh->lhs, recs) && checkWhere(wh->rhs, recs);
        case WHERE_OR:
            return checkWhere(wh->lhs, recs) || checkWhere(wh->rhs, recs);
        case WHERE_NOT:
            return !checkWhere(wh->lhs, recs);
        case WHERE_IS_NOT_NULL:
            return !isNull(wh->lhs);
        case WHERE_IS_NULL:
            return isNull(wh->lhs);
        default:
            if (isNull(wh->rhs))
                return false;
            {
                ExprType vl, vr;
                vl = fetchValue(dynamic_cast<AstCol*>(wh->lhs), recs);
                if (wh->rhs->type == AST_COL)
                    vr = fetchValue(dynamic_cast<AstCol*>(wh->rhs), recs);
                else
                    vr = calcExpr(wh->rhs);
                
                bool vlStr = (vl.type != TYPE_INT) && (vl.type != TYPE_FLOAT);
                bool vrStr = (vr.type != TYPE_INT) && (vr.type != TYPE_FLOAT);
                
                float diff = 0;
                if (vlStr ^ vrStr)
                    throw EvalException("cannot compare string and number");
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