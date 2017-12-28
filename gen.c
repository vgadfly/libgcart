#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "tl.h"
#include "gen.h"
#include "object.h"

static GHashTable *headers;
static GHashTable *sources;
static GList *functions;
static GList *types;

enum wain_type {
    T_TYPE_INT,
    T_TYPE_LONG,
    T_TYPE_UINT,
    T_TYPE_STRING,
    T_TYPE_OBJECT
};

typedef struct _WainArg WainArg;
struct _WainArg {
    gchar *type;
    gchar *name;
    enum wain_type klass;
    int is_list;
};


void yyerror( const char *estr )
{
    fprintf( stderr, "parser error: %s\n", estr );
}

static gchar *wain_class_from_tl( char *type_name, int *klass )
{

    if (!strcmp(type_name, "int")) {
        *klass = T_TYPE_INT;
        return g_strdup("gint32");
    }
    if (!strcmp(type_name, "long")) {
        *klass = T_TYPE_LONG;
        return g_strdup("gint64");
    }
    if (!strcmp(type_name, "string")) {
        *klass = T_TYPE_STRING;
        return g_strdup("gchar *");
    }
    if (!strcmp(type_name, "nat")) {
        *klass = T_TYPE_UINT;
        return g_strdup("guint32");
    }
    *klass = T_TYPE_OBJECT;
    GString *name = g_string_new(type_name);
    g_string_prepend( name, "Wain" );

    return g_string_free( name, 0 );
}

static gchar *wain_type_from_tl( tl_type *type, int *is_list, int *klass )
{
    if (!strcmp(type->name, "Vector")) {
        if (type->modifiers != TYPE_MOD_TEMPLATE){
            fprintf( stderr, "Error in TL: Vector without tepmplate arguments\n" );
            return NULL;
        }
        *is_list = 1;
        return wain_class_from_tl( type->t_args->data, klass );
    }
    *is_list = 0;
    return wain_class_from_tl( type->name, klass );
}

static gchar *wain_var_from_tl( char *arg, int index )
{
    if (!arg || !*arg) {
        GString *s = g_string_new("arg_");
        g_string_append_printf( s, "%d", index );
        return g_string_free( s, 0 );
    }
    return g_strdup(arg);
}

static GList *wain_args_from_tl( tl_list *args )
{
    GList *res = NULL;

    int i;
    for (i=0; args; args = args->next, i++) {
        int is_list, klass;
        tl_arg *arg = args->data;
        gchar *type_name = wain_type_from_tl( arg->type, &is_list, &klass );
        gchar *var_name = wain_var_from_tl( arg->name, i );

        WainArg *wa = g_new(WainArg, 1);
        wa->name = var_name;
        wa->type = type_name;
        wa->klass = klass;
        wa->is_list = is_list;
        res = g_list_append( res, wa );
    }
    return res;
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
    if (!g_hash_table_contains( sources, source_name->str ))
        g_hash_table_insert( sources, g_strdup( source_name->str ), NULL );
    header = fopen(header_name->str, "a+");
    source = fopen(source_name->str, "a+");
    
    fprintf( header, "#define WAIN_TYPE_%s wain_%s_get_type()\n", macro_name, method_prefix );
    fprintf( header, "G_DECLARE_FINAL_TYPE (Wain%s, wain_%s, WAIN, %s, WainObject)\n",
            class_name, method_prefix, macro_name );
    fprintf( header, "\n" );
    fprintf( header, "typedef struct _Wain%s Wain%s;\n", class_name, class_name );
    fprintf( header, "struct _Wain%s {\n", class_name );
    fprintf( header, "  WainObject parent_instance;\n" );

    GList *arglist = wain_args_from_tl( args );
    GList *al;
    for (al = arglist; al; al = al->next) {
        WainArg *wa = al->data;
        if (wa->is_list)
            fprintf( header, "  GList * /* <%s> */ %s;\n", wa->type, wa->name );
        else
            fprintf( header, "  %s %s%s;\n", 
                    wa->type, wa->klass == T_TYPE_OBJECT ? "*" : "", wa->name );
    }

    fprintf( header, "};\n" );
    fprintf( header, "\n" );
    fprintf( header, "void wain_%s_serialize( Wain%s *, WainStream * );\n", method_prefix, class_name );
    fprintf( header, "\n" );

    fprintf( source, "G_DEFINE_TYPE(Wain%s, wain_%s, WAIN_TYPE_OBJECT)\n", class_name, method_prefix );
    fprintf( source, "\n" );
    fprintf( source, "static void wain_%s_init( Wain%s *obj ) {  }\n",
            method_prefix, class_name );

    fprintf( source, "static void wain_%s_class_init( Wain%sClass *klass )\n",
            method_prefix, class_name );
    fprintf( source, "{\n" );
    fprintf( source, "  klass->parent_class.id = 0x%x;\n", hash );
    fprintf( source, "  klass->parent_class.serialize = wain_%s_serialize;\n", method_prefix );
    fprintf( source, "}\n" );

    fprintf( source, "void wain_%s_serialize( Wain%s *self, WainStream *stream )\n", 
            method_prefix, class_name );
    fprintf( source, "{\n" );
    fprintf( source, "  WainObjectClass *woc = WAIN_OBJECT_GET_CLASS(self);\n" );
    fprintf( source, "  wain_int_serialize( woc->id, stream );\n" );
    for (al = arglist; al; al = al->next) {
        WainArg *wa = al->data;
        if (wa->is_list) {
            fprintf( source, "  wain_int_serialize( g_list_length( self->%s ), stream );\n",
                    wa->name );
            fprintf( source, "  GList *l_%s;\n", wa->name );
            fprintf( source, "  for(l_%1$s=self->%1$s; l_%1$s; l_%1$s = l_%1$s->next) {\n",
                    wa->name );
            switch (wa->klass) {
                case T_TYPE_OBJECT:
                    fprintf( source, "    %1$s *%2$s = l_%2$s->data;\n",
                            wa->type, wa->name );
                    break;
                case T_TYPE_STRING:
                    fprintf( source, "    %1$s %2$s = l_%2$s->data;\n",
                            wa->type, wa->name );
                    break;
                default:
                    fprintf( source, "    %1$s %2$s = *(%1$s *)l_%2$s->data;\n",
                            wa->type, wa->name );
            }
            switch (wa->klass) {
                case T_TYPE_INT:
                case T_TYPE_UINT:
                    fprintf( source, "    wain_int_serialize( %s, stream );\n",
                           wa->name );
                    break;
                case T_TYPE_LONG:
                    fprintf( source, "    wain_long_serialize( %s, stream );\n",
                           wa->name );
                    break;
                case T_TYPE_STRING:
                    fprintf( source, "    wain_str_serialize( %s, stream );\n",
                           wa->name );
                    break;
                case T_TYPE_OBJECT:
                    fprintf( source, "    wain_object_serialize( WAIN_OBJECT(%s), stream );\n",
                           wa->name );
                    break;
            }
            fprintf( source, "  }\n" );
        }
        else {
            switch( wa->klass ){
                case T_TYPE_INT:
                case T_TYPE_UINT:
                    fprintf( source, "  wain_int_serialize( self->%s, stream );\n",
                           wa->name );
                    break;
                case T_TYPE_LONG:
                    fprintf( source, "  wain_long_serialize( self->%s, stream );\n",
                           wa->name );
                    break;
                case T_TYPE_STRING:
                    fprintf( source, "  wain_str_serialize( self->%s, stream );\n",
                           wa->name );
                    break;
                case T_TYPE_OBJECT:
                    fprintf( source, "  wain_object_serialize( WAIN_OBJECT(self->%s), stream );\n",
                           wa->name );
                    break;
            }
        }
    }
    fprintf( source, "}\n" );
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
        fprintf( src, "#include \"object.h\"\n" );
        fprintf( src, "\n" );
        GList *k, *keys = g_hash_table_get_keys( headers );
        for ( k = keys; k; k = k->next ){
            fprintf( src, "#include \"%s\"\n", k->data );
        }
        g_list_free(keys);
        fprintf( src, "\n#endif\n" );
        fclose(src);

        src = fopen( "auto.c", "w" );
        fprintf( src, "#include \"auto.h\"\n\n" );
        
        keys = g_hash_table_get_keys( sources );
        for ( k = keys; k; k = k->next ){
            fprintf( src, "#include \"%s\"\n", k->data );
        }
        fclose(src);
    }

    return 0;
}

