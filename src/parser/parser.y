%define api.value.type { class AstBase* }
%{
    #include <cstdlib>
    #include <iostream>
    #include "parser/parser_base.h"
%}

%token DATABASE  TABLE      INDEX    CREATE  DROP
%token DATABASES TABLES     SHOW
%token USE       DESC       SET      INTTYPE CHARTYPE
%token FOREIGN   FLOATTYPE  DATETYPE PRIMARY VARCHARTYPE
%token KEY       REFERENCES NOT      SQLNULL INSERT
%token INTO      VALUES     DELETE   FROM    WHERE
%token UPDATE    SELECT     AND      OR      IS
%token LITERAL   IDENTIFIER
%token '+' '-' ',' '(' ')' '=' '<' '>' UMINUS
%token LESS_EQ   GREATER_EQ NOT_EQ

%left OR
%left AND
%nonassoc '=' NOT_EQ
%nonassoc '<' '>' LESS_EQ GREATER_EQ
%left '+' '-'
%nonassoc UMINUS NOT
%nonassoc '.'
%nonassoc ')'

%start Program

%% 

Program:    StmtList {
                $$ = new AstTopLevel($1);
                
                SemRecorder* sm = new SemRecorder();
                IdentPrinter* ip = new IdentPrinter();
                if (!($$->checkSemantic(*sm)))
                    exit(0);
                $$->printTree(*ip);
                try {
                    if (std::any_cast<bool>($$->eval()))
                        std::cout << std::endl << "*** Database Operation Success ***" << std::endl;
                    else
                        std::cout << std::endl << "*** FAILED ***" << std::endl;
                }
                catch (std::bad_any_cast ex) {
                    std::cout << std::endl << "*** UNEXPECTED ERROR: Contact programmer please ***" << std::endl;
                }
                delete ip;
                delete sm;
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
            SET Ident '=' Value {
                $$ = new AstSetParam($2, $4);
            }|
            CREATE DATABASE Ident {
                $$ = new AstCreateDB($3);
            }|
            DROP DATABASE Ident {
                $$ = new AstDropDB($3);
            }|
            USE Ident {
                $$ = new AstUseDB($2);
            }|
            SHOW TABLES {
                $$ = new AstShowTables();
            }|
            CREATE TABLE Ident '(' FieldList ')' {
                $$ = new AstCreateTable($3, $5);
            }|
            DROP TABLE Ident {
                $$ = new AstDropTable($3);
            }|
            DESC Ident {
                $$ = new AstDesc($2);
            }|
            INSERT INTO Ident VALUES ValLists {
                $$ = new AstInsert($3, $5);
            }|
            DELETE FROM Ident WhereIf {
                $$ = new AstDelete($3, $4);
            }|
            UPDATE Ident SET SetClause WhereIf {
                $$ = new AstUpdate($2, $4, $5);
            }|
            SELECT Selector FROM IdentList WhereIf {
                $$ = new AstSelect($2, $4, $5);
            }|
            CREATE INDEX Ident '(' Ident ')' {
                $$ = new AstCreateIndex($3, $5);
            }|
            DROP INDEX Ident '(' Ident ')' {
                $$ = new AstDropIndex($3, $5);
            };

FieldList:  Field {
                $$ = new AstFieldList(NULL, $1);
            }|
            FieldList ',' Field {
                $$ = new AstFieldList($1, $3);
            };

Field:      Ident Type NOT SQLNULL {
                $$ = new AstField($1, $2, true);
            }|
            Ident Type {
                $$ = new AstField($1, $2, false);
            }|
            PRIMARY KEY '(' IdentList ')' {
                $$ = new AstPrimaryKeyDecl($4);
            }|
            FOREIGN KEY '(' Ident ')' REFERENCES Ident '(' Ident ')' {
                $$ = new AstForeignKeyDecl($4, $7, $9);
            };

Type:       INTTYPE '(' LITERAL ')' {
                $$ = new AstType(TYPE_INT, $3);
            }|
            CHARTYPE '(' LITERAL ')' {
                $$ = new AstType(TYPE_CHAR, $3);
            }|
            VARCHARTYPE '(' LITERAL ')' {
                $$ = new AstType(TYPE_VARCHAR, $3);
            }|
            DATETYPE {
                $$ = new AstType(TYPE_DATE, NULL);
            }|
            FLOATTYPE {
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
            NOT WhClause {
                $$ = new AstWhereClause($2, NULL, WHERE_NOT);
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

Col:        Ident {
                $$ = new AstCol(NULL, $1);
            }|
            Ident '.' Ident {
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

SetSingle:  Ident '=' Value {
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

IdentList:  Ident {
                $$ = new AstIdentList(NULL, $1);
            }|
            IdentList ',' Ident {
                $$ = new AstIdentList($1, $3);
            };

Ident:      IDENTIFIER|
            DATETYPE {
                $$ = new AstIdentifier("date");
            };