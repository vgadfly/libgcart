%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define YYSTYPE char *

extern int yydebug;

void yyerror( const char *estr )
{
    fprintf( stderr, "parser error: %s\n", estr );
}

int yywrap()
{
    return 1;
}

int main(int argc, char *argv[])
{
    if (argc > 1){
        if (!strcmp(argv[1], "-d"))
            yydebug = 1;
    }
    yyparse();
    return 0;
}

enum {
    TL_TYPES,
    TL_FUNCS
} tl_context = TL_TYPES;

char *ns_concat( char *ns, char *id )
{
    char *res = malloc(strlen(ns) + strlen(id) + 2);
    strcpy( res, ns );
    strcat( res, "." );
    strcat( res, id );
    return res;
}

%}

//%define lr.type ielr
%define parse.error verbose

%token NUM LC_ID LC_ID_NS UC_ID UC_ID_NS ID_HASH FUNCTIONS TYPES 
%start program

%left '.'

%%

type-term:  type-id
        | '!' type-id
        | type-id '<' tpar-list '>'
        ;    

tpar-list: type-id
        | tpar-list ',' type-id
        ;

type-id: full-id | '#' ;

full-id:  LC_ID
        | LC_ID '.' LC_ID { $$ = ns_concat( $1, $3 ); free($1); free($3); }
        | UC_ID
        | LC_ID '.' UC_ID { $$ = ns_concat( $1, $3 ); free($1); free($3); }
        ;


///////////////////////////////////////

program: declarations
        {
            printf( "Whole schema parsed\n" );
        }
        ;

declarations: /* empty */
        | declarations declaration
        ;

declaration: combinator
        | functions-separator
        | types-separator
        ;

functions-separator: FUNCTIONS
        {
            tl_context = TL_FUNCS;
            printf("functions!\n");
        }
        ;

types-separator: TYPES
        {
            tl_context = TL_TYPES;
            printf("types!\n");
        }
        ;

combinator: full-id opt-hash opt-args args '=' result-type ';'
        {
            printf("DECL of %s constr %s(%s)\n", $6, $1, $2);
            free($1);
            free($4);
        }
        ;

opt-hash: /* empty */ { $$ = NULL; }
        | ID_HASH
        ;

opt-args: /* empty */
        | '{' args '}'
        ;

args: /* empty */
    | args arg
    ;

arg: LC_ID ':' type-term
        | LC_ID ':' condition '?' type-term
        | UC_ID ':' type-term
        | type-term
        | '[' LC_ID ']'
        ;

condition: LC_ID bit-selector
        ;

bit-selector: /* empty */
        | '.' NUM
        ;

result-type: type-term opt-params;

opt-params: /* empty */
        | opt-params LC_ID
        ;

%%

