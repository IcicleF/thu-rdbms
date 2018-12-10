%option caseless
%{
    #include "parser/parser_base.h"
    #include "parser.tab.hpp"

    std::string builder;
%}

NEWLINE         (\r|\n|\r\n)
DIGIT           ([0-9])
INTEGER_VAL     ({DIGIT}+)
DECIMAL_VAL     ({INTEGER_VAL}"."{INTEGER_VAL})
IDENTIFIER      ([A-Za-z][_0-9A-Za-z]*)
WHITESPACE      ([ \t]+)

%x STR

%%

{NEWLINE}       { }
{WHITESPACE}    { }


"DATABASE"      { yylval = new AstKeyword(DATABASE, "DATABASE"); return DATABASE; }
"DATABASES"     { yylval = new AstKeyword(DATABASES, "DATABASES"); return DATABASES; }
"TABLE"         { yylval = new AstKeyword(TABLE, "TABLE"); return TABLE; }
"TABLES"        { yylval = new AstKeyword(TABLES, "TABLES"); return TABLES; }
"SHOW"          { yylval = new AstKeyword(SHOW, "SHOW"); return SHOW; }
"INDEX"         { yylval = new AstKeyword(INDEX, "INDEX"); return INDEX; }
"CREATE"        { yylval = new AstKeyword(CREATE, "CREATE"); return CREATE; }
"DROP"          { yylval = new AstKeyword(DROP, "DROP"); return DROP; }
"USE"           { yylval = new AstKeyword(USE, "USE"); return USE; }
"DESC"          { yylval = new AstKeyword(DESC, "DESC"); return DESC; }
"SET"           { yylval = new AstKeyword(SET, "SET"); return SET; }
"INT"           { yylval = new AstKeyword(INTTYPE, "INT"); return INTTYPE; }
"CHAR"          { yylval = new AstKeyword(CHARTYPE, "CHAR"); return CHARTYPE; }
"VARCHAR"       { yylval = new AstKeyword(VARCHARTYPE, "VARCHAR"); return VARCHARTYPE; }
"FLOAT"         { yylval = new AstKeyword(FLOATTYPE, "FLOAT"); return FLOATTYPE; }
"DATE"          { yylval = new AstKeyword(DATETYPE, "DATE"); return DATETYPE; }
"PRIMARY"       { yylval = new AstKeyword(PRIMARY, "PRIMARY"); return PRIMARY; }
"FOREIGN"       { yylval = new AstKeyword(FOREIGN, "FOREIGN"); return FOREIGN; }
"KEY"           { yylval = new AstKeyword(KEY, "KEY"); return KEY; }
"REFERENCES"    { yylval = new AstKeyword(REFERENCES, "REFERENCES"); return REFERENCES; }
"NOT"           { yylval = new AstKeyword(NOT, "NOT"); return NOT; }
"NULL"          { yylval = new AstKeyword(SQLNULL, "NULL"); return SQLNULL; }
"INSERT"        { yylval = new AstKeyword(INSERT, "INSERT"); return INSERT; }
"INTO"          { yylval = new AstKeyword(INTO, "INTO"); return INTO; }
"VALUES"        { yylval = new AstKeyword(VALUES, "VALUES"); return VALUES; }
"DELETE"        { yylval = new AstKeyword(DELETE, "DELETE"); return DELETE; }
"FROM"          { yylval = new AstKeyword(FROM, "FROM"); return FROM; }
"WHERE"         { yylval = new AstKeyword(WHERE, "WHERE"); return WHERE; }
"UPDATE"        { yylval = new AstKeyword(UPDATE, "UPDATE"); return UPDATE; }
"SELECT"        { yylval = new AstKeyword(SELECT, "SELECT"); return SELECT; }
"AND"           { yylval = new AstKeyword(AND, "AND"); return AND; }
"OR"            { yylval = new AstKeyword(OR, "OR"); return OR; }
"IS"            { yylval = new AstKeyword(IS, "IS"); return IS; }

{INTEGER_VAL}   { yylval = new AstLiteral(yytext, L_INT); return LITERAL; }
{DECIMAL_VAL}   { yylval = new AstLiteral(yytext, L_DECIMAL); return LITERAL; }

<INITIAL>"'"    { BEGIN(STR); builder.clear(); }
<STR>{NEWLINE}  { yyerror("newline not allowed in string"); }
<STR><<EOF>>    { yyerror("unterminated string"); }
<STR>"'"        { BEGIN(INITIAL); yylval = new AstLiteral(builder.c_str(), L_STRING); return LITERAL; }
<STR>"\\'"      { builder += '\''; }
<STR>"\\\\"     { builder += '\\'; }
<STR>.          { builder += yytext; }

"("             { yylval = new AstOper('('); return '('; }
")"             { yylval = new AstOper(')'); return ')'; }
"+"             { yylval = new AstOper('+'); return '+'; }
"-"             { yylval = new AstOper('-'); return '-'; }
"*"             { yylval = new AstOper('*'); return '*'; }
","             { yylval = new AstOper(','); return ','; }
"="             { yylval = new AstOper('='); return '='; }
"<"             { yylval = new AstOper('<'); return '<'; }
">"             { yylval = new AstOper('>'); return '>'; }
";"             { yylval = new AstOper(';'); return ';'; }
"."             { yylval = new AstOper('.'); return '.'; }
"<="            { yylval = new AstOper(LESS_EQ, "<="); return LESS_EQ; }
">="            { yylval = new AstOper(GREATER_EQ, ">="); return GREATER_EQ; }
"<>"            { yylval = new AstOper(NOT_EQ, "<>"); return NOT_EQ; }

{IDENTIFIER}    { yylval = new AstIdentifier(yytext); return IDENTIFIER; }

.               { issueError(yytext); }

%%

int yywrap() {
    return 1;
}