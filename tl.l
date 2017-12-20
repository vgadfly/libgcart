%{

#include <stdio.h>
#include <stdlib.h>

#include "tl.h"
#include "tl.tab.h"

%}

%%

---functions---         return FUNCTIONS;
---types---             return TYPES;
[a-z][a-zA-Z0-9_]*      yylval.string = strdup(yytext); return LC_ID;
[A-Z][a-zA-Z0-9_]*      yylval.string = strdup(yytext); return UC_ID;
[0-9]+                  yylval.number = atoi(yytext); return NUM;
#[0-9a-fA-F]{1,8}       yylval.number = strtol(yytext+1, NULL, 16); return ID_HASH;

\n                      /* skip new line */
[ \t]+                  /* skip spaces */
\/\/.*$                 /* skip comments */
\/\*.*\*\/              /* skip comments */
.                       return (int)yytext[0];

%%

