%define api.value.type { class AstBase* }
%{
    #include "parser_base.h"
%}

%token DATABASE  TABLE      INDEX  CREATE  DROP
%token DATABASES TABLES     SHOW
%token USE       DESC       SET    INT     CHAR
%token VARCHAR   FLOAT      DATE   PRIMARY FOREIGN
%token KEY       REFERENCES NOT    SQLNULL INSERT
%token INTO      VALUES     DELETE FROM    WHERE
%token UPDATE    SELECT     AND    OR      IS
%token INTEGER   IDENTIFIER STRING LITERAL
%token '+' '-' ',' '(' ')' '=' '<' '>' UMINUS
%token LESS_EQ   GREATER_EQ NOT_EQ

%left OR
%left AND
%nonassoc '=' NOT_EQ
%nonassoc '<' '>' LESS_EQ GREATER_EQ
%left '+' '-'
%nonassoc UMINUS
%nonassoc '.'
%nonassoc ')'

%start Program

%% 

Program:    StmtList {
                $$ = new AstTopLevel($1);
                std::cout << std::endl;
                IdentPrinter* ip = new IdentPrinter();
                $$->printTree(*ip);
                delete ip;
                std::cout << std::endl;
            };

StmtList:   Stmt ';' {
                $$ = new AstStmtList(NULL, $1);
            }|
            StmtList Stmt ';' {
                $$ = new AstStmtList($1, $2);
            };

Stmt:       SHOW DATABASES {
                $$ = new AstShowDB();
            }|
            SET IDENTIFIER '=' Value {
                $$ = new AstSetParam($2, $4);
            }|
            CREATE DATABASE IDENTIFIER {
                $$ = new AstCreateDB($3);
            }|
            DROP DATABASE IDENTIFIER {
                $$ = new AstDropDB($3);
            }|
            USE IDENTIFIER {
                $$ = new AstUseDB($2);
            }|
            SHOW TABLES {
                $$ = new AstShowTables();
            }|
            CREATE TABLE IDENTIFIER '(' FieldList ')' {
                $$ = new AstCreateTable($3, $5);
            }|
            DROP TABLE IDENTIFIER {
                $$ = new AstDropTable($3);
            }|
            DESC IDENTIFIER {
                $$ = new AstDesc($2);
            }|
            INSERT INTO IDENTIFIER VALUES ValLists {
                $$ = new AstInsert($3, $5);
            }|
            DELETE FROM IDENTIFIER WhereIf {
                $$ = new AstDelete($3, $4);
            }|
            UPDATE IDENTIFIER SET SetClause WhereIf {
                $$ = new AstUpdate($2, $4, $5);
            }|
            SELECT Selector FROM IdentList WhereIf {
                $$ = new AstSelect($2, $4, $5);
            }|
            CREATE INDEX IDENTIFIER '(' IDENTIFIER ')' {
                $$ = new AstCreateIndex($3, $5);
            }|
            DROP INDEX IDENTIFIER '(' IDENTIFIER ')' {
                $$ = new AstDropIndex($3, $5);
            };

FieldList:  Field {
                $$ = new AstFieldList(NULL, $1);
            }|
            FieldList ',' Field {
                $$ = new AstFieldList($1, $3);
            };

Field:      IDENTIFIER Type NOT SQLNULL {
                $$ = new AstField($1, $2, true);
            }|
            IDENTIFIER Type {
                $$ = new AstField($1, $2, false);
            }|
            PRIMARY KEY '(' IdentList ')' {
                $$ = new AstPrimaryKeyDecl($4);
            }|
            FOREIGN KEY '(' IDENTIFIER ')' REFERENCES IDENTIFIER '(' IDENTIFIER ')' {
                $$ = new AstForeignKeyDecl($4, $7, $9);
            };

Type:       INT '(' LITERAL ')' {
                $$ = new AstType(TYPE_INT, $3);
            }|
            CHAR '(' LITERAL ')' {
                $$ = new AstType(TYPE_CHAR, $3);
            }|
            VARCHAR '(' LITERAL ')' {
                $$ = new AstType(TYPE_VARCHAR, $3);
            }|
            DATE {
                $$ = new AstType(TYPE_DATE, NULL);
            }|
            FLOAT {
                $$ = new AstType(TYPE_FLOAT, NULL);
            };

ValLists:   '(' ValList ')' {
                $$ = new AstValLists(NULL, $2);
            }|
            ValLists ',' '(' ValList ')' {
                $$ = new AstValLists($1, $4);
            };

ValList:    Value {
                $$ = new AstValList(NULL, $1);
            }|
            ValList ',' Value {
                $$ = new AstValList($1, $3);
            };

Value:      LITERAL|
            SQLNULL;

WhereIf:    WHERE WhClause {
                $$ = $2;
            }|
            /* empty */ {
                $$ = NULL;
            };

WhClause:   WhClause AND WhClause {
                $$ = new AstWhereClause($1, $3, WHERE_AND);
            }|
            WhClause OR WhClause {
                $$ = new AstWhereClause($1, $3, WHERE_OR);
            }|
            '(' WhClause ')' {
                $$ = $2;
            }|
            Col '=' Expr {
                $$ = new AstWhereClause($1, $3, WHERE_EQ);
            }|
            Col '<' Expr {
                $$ = new AstWhereClause($1, $3, WHERE_LT);
            }|
            Col '>' Expr {
                $$ = new AstWhereClause($1, $3, WHERE_GT);
            }|
            Col NOT_EQ Expr {
                $$ = new AstWhereClause($1, $3, WHERE_NE);
            }|
            Col LESS_EQ Expr {
                $$ = new AstWhereClause($1, $3, WHERE_LE);
            }|
            Col GREATER_EQ Expr {
                $$ = new AstWhereClause($1, $3, WHERE_GE);
            }|
            Col IS SQLNULL {
                $$ = new AstWhereClause($1, NULL, WHERE_IS_NULL);
            }|
            Col IS NOT SQLNULL {
                $$ = new AstWhereClause($1, NULL, WHERE_IS_NOT_NULL);
            };

Col:        IDENTIFIER {
                $$ = new AstCol(NULL, $1);
            }|
            IDENTIFIER '.' IDENTIFIER {
                $$ = new AstCol($1, $3);
            };

Expr:       Expr '+' Expr {
                $$ = new AstExpr($1, $3, EXPR_ADD);
            }|
            Expr '-' Expr {
                $$ = new AstExpr($1, $3, EXPR_SUB);
            }|
            '-' Expr    %prec UMINUS {
                $$ = new AstExpr($1, NULL, EXPR_UMINUS);
            }|
            '(' Expr ')' {
                $$ = $2;
            }|
            Value|
            Col;

SetClause:  SetSingle {
                $$ = new AstSetClause(NULL, $1);
            }|
            SetClause ',' SetSingle {
                $$ = new AstSetClause($1, $3);
            };

SetSingle:  IDENTIFIER '=' Value {
                $$ = new AstSet($1, $3);
            };

Selector:   '*' {
                $$ = new AstSelector(NULL);
            }|
            ColList {
                $$ = new AstSelector($1);
            };

ColList:    Col {
                $$ = new AstColList(NULL, $1);
            }|
            ColList ',' Col {
                $$ = new AstColList($1, $3);
            };

IdentList:  IDENTIFIER {
                $$ = new AstIdentList(NULL, $1);
            }|
            IdentList ',' IDENTIFIER {
                $$ = new AstIdentList($1, $3);
            };