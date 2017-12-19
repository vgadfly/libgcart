%{

#include <stdio.h>

#define YYSTYPE char *

#include "tl.tab.h"


%}

%%

---functions---         return FUNCTIONS;
---types---             return TYPES;
[a-z][a-zA-Z0-9_]*      yylval = strdup(yytext); return LC_ID;
[A-Z][a-zA-Z0-9_]*      yylval = strdup(yytext); return UC_ID;
[0-9]+                  yylval = strdup(yytext); return NUM;
#[0-9a-fA-F]{1,8}       yylval = strdup(yytext+1); return ID_HASH;

\n                      /* skip new line */
[ \t]+                  /* skip spaces */
\/\/.*$                 /* skip comments */
\/\*.*\*\/              /* skip comments */
.                       return (int)yytext[0];

%%

