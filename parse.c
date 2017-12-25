#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "tl.h"

void yyerror( const char *estr )
{
    fprintf( stderr, "parser error: %s\n", estr );
}

void tl_type_gen( char *constr, int hash, tl_type *type, tl_list *args )
{
    printf( "TYPE %s constr %s(%08x) { ", type->name, constr, hash );
    for(; args; args = args->next){
        tl_arg *arg = args->data;
        printf( "%s: %s, ", arg->name, arg->type->name );
    }
    printf("}\n");
}

void tl_func_gen( char *name, int hash, tl_type *res, tl_list *args )
{
    printf( "FUNC %s %s(%08x) { ", res->name, name, hash );
    for(; args; args = args->next){
        tl_arg *arg = args->data;
        printf( "%s: %s, ", arg->name, arg->type->name );
    }
    printf("}\n");
}

extern int yydebug;
int main( int argc, char *argv[] )
{
    if (argc > 1){
        if (!strcmp(argv[1], "-d"))
            yydebug = 1;
    }
    yyparse();
    return 0;
}

