%option caseless
%{
    #include "parser_base.h"
    #include "parser.tab.h"
%}

NEWLINE         (\r|\n|\r\n)
DIGIT           ([0-9])
INTEGER         ({DIGIT}+)
DECIMAL         ({INTEGER}"."{INTEGER}"f")
IDENTIFIER      ([A-Za-z][_0-9A-Za-z]*)
STRING          ('[^']*')
WHITESPACE      ([ \t]+)

%%

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
"INT"           { yylval = new AstKeyword(INT, "INT"); return INT; }
"CHAR"          { yylval = new AstKeyword(CHAR, "CHAR"); return CHAR; }
"VARCHAR"       { yylval = new AstKeyword(VARCHAR, "VARCHAR"); return VARCHAR; }
"FLOAT"         { yylval = new AstKeyword(FLOAT, "FLOAT"); return FLOAT; }
"DATE"          { yylval = new AstKeyword(DATE, "DATE"); return DATE; }
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

{INTEGER}       { yylval = new AstLiteral(yytext, L_INT); return LITERAL; }
{DECIMAL}       { yylval = new AstLiteral(yytext, L_DECIMAL); return LITERAL; }
{STRING}        { yylval = new AstLiteral(yytext, L_STRING); return LITERAL; }

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
"<="            { yylval = new AstOper(LESS_EQ); return LESS_EQ; }
">="            { yylval = new AstOper(GREATER_EQ); return GREATER_EQ; }
"<>"            { yylval = new AstOper(NOT_EQ); return NOT_EQ; }

{IDENTIFIER}    { yylval = new AstIdentifier(yytext); return IDENTIFIER; }

{NEWLINE}       { }
{WHITESPACE}    { }

.               { issueError(yytext); }

%%

int yywrap() {
    return 1;
}