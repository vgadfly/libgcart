%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tl.h"

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

static char *ns_concat( char *ns, char *id )
{
    char *res = malloc(strlen(ns) + strlen(id) + 2);
    strcpy( res, ns );
    strcat( res, "." );
    strcat( res, id );
    return res;
}

static tl_type *tl_type_new( char *name, int modifiers )
{
    tl_type *res = malloc(sizeof(tl_type));
    res->name = name;
    res->modifiers = modifiers;
    return res;
}

static tl_type *tl_type_new_template( char *name, GList *targs )
{
    tl_type *res = tl_type_new( name, TYPE_MOD_TEMPLATE );
    res->t_args = targs;
    return res;
}

static tl_arg *tl_arg_new( char *name, tl_type *type )
{
    tl_arg *arg = malloc(sizeof(tl_arg));
    arg->name = name;
    arg->type = type;
    arg->modifiers = ARG_MOD_NONE;
    return arg;
}

static tl_arg *tl_arg_new_cond( char *name, tl_type *type, tl_cond *cond )
{
    tl_arg *arg = tl_arg_new( name, type );
    arg->modifiers = ARG_MOD_COND;
    arg->cond_field = cond->name;
    arg->cond_bitmask = cond->bitmask;
    return arg;
}

static tl_arg *tl_arg_new_mult( char *name, tl_type *type )
{
    tl_arg *arg = tl_arg_new( name, type );
    arg->modifiers = ARG_MOD_MULT;
    return arg;
}

static tl_cond *tl_cond_new( char *name, int bitmask )
{
    tl_cond *cond = malloc(sizeof(tl_cond));
    cond->name = name;
    cond->bitmask = bitmask;
    return cond;
}

static void tl_type_gen( char *constr, int hash, tl_type *type, GList *args )
{
    printf( "TYPE %s constr %s(%08x) { ", type->name, constr, hash );
    for(; args; args = args->next){
        tl_arg *arg = args->data;
        printf( "%s: %s, ", arg->name, arg->type->name );
    }
    printf("}\n");
}

%}

//%define lr.type ielr
%define parse.error verbose

%union {
    int number;
    char *string;
    GList *list;
    tl_arg *arg;
    tl_type *type;
    tl_cond *cond;
}

%token <number> NUM
%token <string> LC_ID
%token <string> UC_ID
%token <number> ID_HASH 
%token FUNCTIONS TYPES 

%type <string> full-id
%type <number> opt-hash
%type <string> type-id
%type <type> type-term
%type <type> result-type
%type <list> tpar-list
%type <list> args
%type <arg> arg
%type <cond> condition
%type <number> bit-selector

%start program

%left '.'

%%

type-term:  type-id { $$ = tl_type_new( $1, TYPE_MOD_NONE ); }
        | '!' type-id { $$ = tl_type_new( $2, TYPE_MOD_BANG ); }
        | type-id '<' tpar-list '>' { $$ = tl_type_new_template( $1, $3 ); }
        ;    

tpar-list: type-id { $$ = g_list_append( NULL, $1 ); }
        | tpar-list ',' type-id { $$ = g_list_append( $1, $3 ); }
        ;

type-id: full-id | '#' { $$ = strdup("nat"); } ;

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
            if (tl_context == TL_TYPES)
                tl_type_gen( $1, $2, $6, $4 );
        }
        ;

opt-hash: /* empty */ { $$ = 0; }
        | ID_HASH
        ;

opt-args: /* empty */
        | '{' args '}'
        ;

args: /* empty */ { $$ = NULL; }
    | args arg { $$ = g_list_append( $1, $2 ); }
    ;

arg: LC_ID ':' type-term { $$ = tl_arg_new( $1, $3 ); }
        | LC_ID ':' condition '?' type-term { $$ = tl_arg_new_cond( $1, $5, $3 ); free($3); }
        | UC_ID ':' type-term { $$ = tl_arg_new( $1, $3 ); }
        | type-term { $$ = tl_arg_new( "", $1 ); }
        | '[' LC_ID ']' { $$ = tl_arg_new_mult( "", tl_type_new( $2, TYPE_MOD_NONE ) ); }
        ;

condition: LC_ID bit-selector { $$ = tl_cond_new( $1, $2 ); }
        ;

bit-selector: /* empty */ { $$ = -1; }
        | '.' NUM { $$ = 1 << $2; }
        ;

result-type: type-term opt-params;

opt-params: /* empty */
        | opt-params LC_ID
        ;

%%

