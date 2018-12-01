%option noyywrap
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

"DATABASE"      { yylval = new AstKeyword(DATABASE); return DATABASE; }
"DATABASES"     { yylval = new AstKeyword(DATABASES); return DATABASES; }
"TABLE"         { yylval = new AstKeyword(TABLE); return TABLE; }
"TABLES"        { yylval = new AstKeyword(TABLES); return TABLES; }
"SHOW"          { yylval = new AstKeyword(SHOW); return SHOW; }
"INDEX"         { yylval = new AstKeyword(INDEX); return INDEX; }
"CREATE"        { yylval = new AstKeyword(CREATE); return CREATE; }
"DROP"          { yylval = new AstKeyword(DROP); return DROP; }
"USE"           { yylval = new AstKeyword(USE); return USE; }
"DESC"          { yylval = new AstKeyword(DESC); return DESC; }
"SET"           { yylval = new AstKeyword(SET); return SET; }
"INT"           { yylval = new AstKeyword(INT); return INT; }
"CHAR"          { yylval = new AstKeyword(CHAR); return CHAR; }
"VARCHAR"       { yylval = new AstKeyword(VARCHAR); return VARCHAR; }
"FLOAT"         { yylval = new AstKeyword(FLOAT); return FLOAT; }
"DATE"          { yylval = new AstKeyword(DATE); return DATE; }
"PRIMARY"       { yylval = new AstKeyword(PRIMARY); return PRIMARY; }
"FOREIGN"       { yylval = new AstKeyword(FOREIGN); return FOREIGN; }
"KEY"           { yylval = new AstKeyword(KEY); return KEY; }
"REFERENCES"    { yylval = new AstKeyword(REFERENCES); return REFERENCES; }
"NOT"           { yylval = new AstKeyword(NOT); return NOT; }
"NULL"          { yylval = new AstKeyword(SQLNULL); return SQLNULL; }
"INSERT"        { yylval = new AstKeyword(INSERT); return INSERT; }
"INTO"          { yylval = new AstKeyword(INTO); return INTO; }
"VALUES"        { yylval = new AstKeyword(VALUES); return VALUES; }
"DELETE"        { yylval = new AstKeyword(DELETE); return DELETE; }
"FROM"          { yylval = new AstKeyword(FROM); return FROM; }
"WHERE"         { yylval = new AstKeyword(WHERE); return WHERE; }
"UPDATE"        { yylval = new AstKeyword(UPDATE); return UPDATE; }
"SELECT"        { yylval = new AstKeyword(SELECT); return SELECT; }
"AND"           { yylval = new AstKeyword(AND); return AND; }
"OR"            { yylval = new AstKeyword(OR); return OR; }
"IS"            { yylval = new AstKeyword(IS); return IS; }

{INTEGER}       { yylval = new AstLiteral(yytext, L_INT); return LITERAL; }
{DECIMAL}       { yylval = new AstLiteral(yytext, L_DECIMAL); return LITERAL; }
{STRING}        { yylval = new AstLiteral(yytext, L_STRING); return LITERAL; }

"("             { yylval = new AstKeyword('('); return '('; }
")"             { yylval = new AstKeyword(')'); return ')'; }
"+"             { yylval = new AstKeyword('+'); return '+'; }
"-"             { yylval = new AstKeyword('-'); return '-'; }
"*"             { yylval = new AstKeyword('*'); return '*'; }
","             { yylval = new AstKeyword(','); return ','; }
"="             { yylval = new AstKeyword('='); return '='; }
"<"             { yylval = new AstKeyword('<'); return '<'; }
">"             { yylval = new AstKeyword('>'); return '>'; }
";"             { yylval = new AstKeyword(';'); return ';'; }
"."             { yylval = new AstKeyword('.'); return '.'; }
"<="            { yylval = new AstKeyword(LESS_EQ); return LESS_EQ; }
">="            { yylval = new AstKeyword(GREATER_EQ); return GREATER_EQ; }
"<>"            { yylval = new AstKeyword(NOT_EQ); return NOT_EQ; }

{IDENTIFIER}    { yylval = new AstIdentifier(yytext); return IDENTIFIER; }

{NEWLINE}       { }
{WHITESPACE}    { }

.               { issueError(yytext); }

%%