#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "tl.h"
#include "gen.h"

static GHashTable *headers;
static GHashTable *sources;
static GList *functions;
static GList *types;

void yyerror( const char *estr )
{
    fprintf( stderr, "parser error: %s\n", estr );
}

void tl_class_gen( char *name, int hash, tl_type *res, tl_list *args )
{
    FILE *header, *source;
    GString *header_name, *source_name;
    gchar **api_name = g_strsplit( name, ".", 2 );
    gchar *macro_name, *method_prefix, *class_name, *temp;

    if (g_strv_length(api_name) == 1) {
        api_name[0][0] = g_ascii_toupper(api_name[0][0]);

        header_name = g_string_new("auto/glob.h");
        source_name = g_string_new("auto/glob.c");

        macro_name = g_ascii_strup( *api_name, -1 );
        method_prefix = g_ascii_strdown( *api_name, -1 );
        class_name = g_strdup( *api_name );
    } else
    if (g_strv_length(api_name) == 2) {
        api_name[0][0] = g_ascii_toupper(api_name[0][0]);
        api_name[1][0] = g_ascii_toupper(api_name[1][0]);

        temp = g_ascii_strdown( api_name[0], -1 );
        source_name = g_string_new( temp );
        header_name = g_string_new( temp );
        g_string_prepend( header_name, "auto/" );
        g_string_prepend( source_name, "auto/" );
        g_string_append( header_name, ".h" );
        g_string_append( source_name, ".c" );
        g_free(temp);

        gchar *ns_uc = g_ascii_strup( api_name[0], -1 );
        gchar *ns_lc = g_ascii_strdown( api_name[0], -1 );
        gchar *fun_uc = g_ascii_strup( api_name[1], -1 );
        gchar *fun_lc = g_ascii_strdown( api_name[1], -1 );

        GString *tstr = g_string_new(NULL);
        g_string_printf( tstr, "%s_%s", ns_uc, fun_uc );
        macro_name = g_string_free( tstr, 0 );
        
        tstr = g_string_new(NULL);
        g_string_printf( tstr, "%s_%s", ns_lc, fun_lc );
        method_prefix = g_string_free( tstr, 0 );
        
        tstr = g_string_new(NULL);
        g_string_printf( tstr, "%s%s", api_name[0], api_name[1] );
        class_name = g_string_free( tstr, 0 );
        
        g_free(ns_uc);
        g_free(ns_lc);
        g_free(fun_uc);
        g_free(fun_lc);
    } else
        goto _return;

    int i;
    for (i = 0; i < sizeof(gen_blacklist)/sizeof(char *); i++) {
        if (!g_strcmp0( class_name, gen_blacklist[i] ))
            goto _free;
    }
    
    if (!g_hash_table_contains( headers, header_name->str ))
        g_hash_table_insert( headers, g_strdup( header_name->str ), NULL );
    header = fopen(header_name->str, "a+");
    source = fopen(source_name->str, "a+");
    
    fprintf( header, "#define WAIN_TYPE_%s wain_%s_get_type()\n", macro_name, method_prefix );
    fprintf( header, "#define WAIN_%s(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), WAIN_TYPE_%s, Wain%s))\n",
            macro_name, macro_name, class_name );
    fprintf( header, "#define WAIN_IS_%s(obj) (G_CHECK_TYPE_INSTANCE_TYPE ((obj), WAIN_TYPE_%s))\n",
            macro_name, macro_name );
    fprintf( header, "#define WAIN_%s_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), WAIN_TYPE_%s, Wain%s))\n",
            macro_name, macro_name, class_name );
    fprintf( header, "#define WAIN_IS_%s_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), WAIN_TYPE_%s))\n",
            macro_name, macro_name );
    fprintf( header, "#define WAIN_%s_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), WAIN_TYPE_%s, Wain%s))\n",
            macro_name, macro_name, class_name );
    fprintf( header, "\n" );
    fprintf( header, "typedef struct _Wain%s Wain%s;\n", class_name, class_name );
    fprintf( header, "struct _Wain%s {\n", class_name );
    fprintf( header, "  WainObject parent_instance;\n" );
    fprintf( header, "};\n" );
    fprintf( header, "struct _Wain%sClass {\n", class_name );
    fprintf( header, "  WainObject parent_class;\n" );
    fprintf( header, "};\n" );
    fprintf( header, "\n" );

    fprintf( source, "#include \"%s\"\n", header_name->str );
    fprintf( source, "G_DEFINE_TYPE(Wain%s, wain_%s, G_TYPE_OBJECT)\n", class_name, method_prefix );
    fprintf( source, "\n" );

    fclose(header);
    fclose(source);
_free:
    g_free(method_prefix);
    g_free(class_name);
    g_free(macro_name);
    g_string_free( header_name, 1 );
    g_string_free( source_name, 1 );
_return:
    g_strfreev(api_name);
}

void tl_type_gen( char *constr, int hash, tl_type *type, tl_list *args )
{
    tl_class_gen( constr, hash, type, args );
}

void tl_func_gen( char *name, int hash, tl_type *res, tl_list *args )
{
    tl_class_gen( name, hash, res, args );
}

    
extern int yydebug;
extern FILE *yyin;
int main( int argc, char *argv[] )
{
    if (argc > 1){
        if (!strcmp(argv[1], "-d"))
            yydebug = 1;
        else
            yyin = fopen(argv[1], "r");
    }
    sources = g_hash_table_new( g_str_hash, g_str_equal );
    headers = g_hash_table_new( g_str_hash, g_str_equal );
    yyparse();
    if (headers && sources){
        FILE *src = fopen( "auto.h", "w" );

        fprintf( src, "#ifndef __WAIN_AUTO_H__\n" );
        fprintf( src, "#define __WAIN_AUTO_H__\n" );
        fprintf( src, "#include <glib-object.h>\n" );
        fprintf( src, "#include <glib.h>\n" );
        fprintf( src, "\n" );
        GList *k, *keys = g_hash_table_get_keys( headers );
        for ( k = keys; k; k = k->next ){
            fprintf( src, "#include \"%s\"\n", k->data );
        }
        g_list_free(keys);
        fprintf( src, "\n#endif\n" );
        fclose(src);
    }

    return 0;
}

