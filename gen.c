#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "tl.h"
#include "gen.h"
#include "object.h"

static GHashTable *headers;
static GHashTable *sources;
static GHashTable *constructors;
static GList *functions;
static GList *types;

enum wain_type {
    T_TYPE_INT,
    T_TYPE_LONG,
    T_TYPE_UINT,
    T_TYPE_STRING,
    T_TYPE_DOUBLE,
    T_TYPE_BOOL,
    T_TYPE_OBJECT
};

const char *wain_type_names[] = {
    "int",
    "long",
    "nat",
    "str",
    "double",
    "bool",
    "object"
};

const char *wain_prop_types[] = {
    "int",
    "uint64",
    "uint",
    "string",
    "double",
    "boolean",
    "object"
};


typedef struct _WainArg WainArg;
struct _WainArg {
    gchar *type;
    gchar *name;
    gchar *uc_name;
    enum wain_type klass;
    int is_list;
    gchar *cond_field;
    int cond_bits;
};

#define VECTOR_ID 0x1cb5c415

void yyerror( const char *estr )
{
    fprintf( stderr, "parser error: %s\n", estr );
}

static gchar *wain_class_from_tl( char *type_name, int *klass )
{
    /* XXX: hashmap? */
    if (!strcmp(type_name, "int")) {
        *klass = T_TYPE_INT;
        return g_strdup("gint32");
    }
    if (!strcmp(type_name, "long")) {
        *klass = T_TYPE_LONG;
        return g_strdup("gint64");
    }
    if (!strcmp(type_name, "string") || !strcmp(type_name, "bytes")) {
        *klass = T_TYPE_STRING;
        return g_strdup("gchar *");
    }
    if (!strcmp(type_name, "nat")) {
        *klass = T_TYPE_UINT;
        return g_strdup("guint32");
    }
    if (!strcmp(type_name, "double")) {
        *klass = T_TYPE_DOUBLE;
        return g_strdup("gdouble");
    }
    if (!strcmp(type_name, "Bool")) {
        *klass = T_TYPE_BOOL;
        return g_strdup("gboolean");
    }

    *klass = T_TYPE_OBJECT;
    GString *name = g_string_new(type_name);
    g_string_prepend( name, "Wain" );

    /* XXX: leaks */
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
        gchar *uc_name = g_ascii_strup( arg->name, -1 );

        WainArg *wa = g_new(WainArg, 1);
        wa->name = var_name;
        wa->uc_name = uc_name;
        wa->type = type_name;
        wa->klass = klass;
        wa->is_list = is_list;

        if ( arg->modifiers & ARG_MOD_COND ) {
            printf( "contitional %s: %s & %d\n", wa->name, arg->cond_field, arg->cond_bitmask );
            wa->cond_field = arg->cond_field;
            wa->cond_bits = arg->cond_bitmask;
        }
        else {
            wa->cond_field = NULL;
        }
        res = g_list_append( res, wa );
    }
    return res;
}

/* helpers */

static void mk_list_item( FILE *output, const char * prefix, enum wain_type klass, const char *ctype, const char *cname )
{
    switch (klass) {
        case T_TYPE_OBJECT:
            fprintf( output, "%s%s *%s = l_%s->data;\n",
                    prefix, ctype, cname, cname );
            break;
        case T_TYPE_STRING:
            fprintf( output, "%s%s %s = l_%s->data;\n",
                    prefix, ctype, cname, cname );
            break;
        default:
            fprintf( output, "%s%s %s = *(%s *)l_%s->data;\n",
                    prefix, ctype, cname, ctype, cname );
    }
}

static void mk_list_item_len( FILE *output, const char *prefix, const char *var, enum wain_type klass, const char *cname )
{
    if (klass < T_TYPE_OBJECT ) {
        fprintf( output, "%s%s += wain_%s_length(%s);\n",
                prefix, var, wain_type_names[klass], cname );
    }
    else {
        fprintf( output, "%s%s += wain_object_length( WAIN_OBJECT(%s) );\n",
                prefix, var, cname );
    }
}

static void mk_list_item_ser( FILE *output, const char *prefix, const char *var, enum wain_type klass, const char *cname )
{
    if (klass < T_TYPE_OBJECT ) {
        fprintf( output, "%swain_%s_serialize( %s, %s );\n",
                prefix, wain_type_names[klass], cname, var );
        
    } 
    else {
        fprintf( output, "%swain_object_serialize( WAIN_OBJECT(%s), %s );\n",
                prefix, cname, var );
    }
}

static void mk_param_len( FILE *output, const char *prefix, const char *var, enum wain_type klass, const char *cname )
{
    if (klass < T_TYPE_OBJECT ) {
            fprintf( output, "%s%s += wain_%s_length(self->%s);\n",
                prefix, var, wain_type_names[klass], cname );
    }
    else {
        fprintf( output, "%s%s += wain_object_length( WAIN_OBJECT(self->%s) );\n",
                prefix, var, cname );
    }
}

static void mk_param_ser( FILE *output, const char *prefix, const char *var, enum wain_type klass, const char *cname )
{
    if (klass < T_TYPE_OBJECT ) {
        fprintf( output, "%swain_%s_serialize( self->%s, %s );\n",
                prefix, wain_type_names[klass], cname, var );
    }
    else {
        fprintf( output, "%swain_object_serialize( WAIN_OBJECT(self->%s), bytes );\n",
                prefix, cname );
    }
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
   
    g_hash_table_insert( constructors, GINT_TO_POINTER(hash), g_strdup(method_prefix) );
    if (!g_hash_table_contains( headers, header_name->str ))
        g_hash_table_insert( headers, g_strdup( header_name->str ), NULL );
    if (!g_hash_table_contains( sources, source_name->str ))
        g_hash_table_insert( sources, g_strdup( source_name->str ), NULL );
    header = fopen(header_name->str, "a+");
    source = fopen(source_name->str, "a+");
   
    /* === header === */

    fprintf( header, "#define WAIN_TYPE_%s wain_%s_get_type()\n", macro_name, method_prefix );
    fprintf( header, "G_DECLARE_FINAL_TYPE (Wain%s, wain_%s, WAIN, %s, WainObject)\n\n",
            class_name, method_prefix, macro_name );
     
    fprintf( header, "typedef struct _Wain%s Wain%s;\n", class_name, class_name );
    fprintf( header, "struct _Wain%s {\n", class_name );
    fprintf( header, "  WainObject parent;\n" );
 
    GList *arglist = wain_args_from_tl( args );
    GList *al;
    
    for (al = arglist; al; al = al->next) {
        WainArg *wa = al->data;
        if (wa->is_list)
            fprintf( header, "  GList * /" "* <%s> *" "/ %s;\n", wa->type, wa->name );
        else
            fprintf( header, "  %s %s%s;\n", 
                    wa->type, wa->klass == T_TYPE_OBJECT ? "*" : "", wa->name );
    }

    fprintf( header, "};\n\n" );
    fprintf( header, "Wain%s *wain_%s_from_bytes( gchar * );\n", class_name, method_prefix );
    
    /* === source === */
    
    fprintf( source, "G_DEFINE_TYPE(Wain%s, wain_%s, WAIN_TYPE_OBJECT)\n\n", class_name, method_prefix );
    /*
    fprintf( source, "enum {\n" );
    fprintf( source, "  PROP_RESERVED,\n" );
    for (al = arglist; al; al = al->next) {
        WainArg *wa = al->data;
        fprintf( source, "  PROP_%s,\n" );
    }
    fprintf( source, "  N_PROPS\n}\n\n" );
    fprintf( source, "static GParamSpec *obj_props[N_PROPS] = { NULL };\n" );
    */
    fprintf( source, "static gint32 wain_%s_length( Wain%s * );\n", method_prefix, class_name );
    fprintf( source, "static void wain_%s_serialize( Wain%s *, gchar * );\n", method_prefix, class_name );
    fprintf( source, "\n" );

    fprintf( source, "static void wain_%s_init( Wain%s *obj ) {  }\n\n",
            method_prefix, class_name );

    fprintf( source, "static void wain_%s_class_init( Wain%sClass *klass )\n",
            method_prefix, class_name );
    fprintf( source, "{\n" );
    fprintf( source, "  klass->parent_class.id = 0x%x;\n", hash );
    fprintf( source, "  klass->parent_class.tl_name = \"%s\";\n", res->name );
    fprintf( source, "  klass->parent_class.serialize = wain_%s_serialize;\n", method_prefix );
    fprintf( source, "  klass->parent_class.length = wain_%s_length;\n", method_prefix );
    /*
    for (al = arglist; al; al = al->next) {
        WainArg *wa = al->data;
        fprintf( source, "  obj_props[PROP_%s] = \n", wa->uc_name );
    }
    */
    fprintf( source, "}\n\n" );
 
    /* === length === */

    fprintf( source, "gint32 wain_%s_length( Wain%s *self )\n", 
            method_prefix, class_name );
    fprintf( source, "{\n" );
    fprintf( source, "  g_return_val_if_fail( WAIN_IS_%s(self), 0 );\n", macro_name );
    fprintf( source, "  gint32 len = 0;\n" );
    fprintf( source, "  WainObjectClass *woc = WAIN_OBJECT_GET_CLASS(self);\n" );
    fprintf( source, "  len += wain_int_length( woc->id );\n" );
    
    int indent;
    static const char *indent_val[] = {"", "  ", "    ", "      ", "        "};

    for (al = arglist; al; al = al->next) {
        indent = 1;
        WainArg *wa = al->data;
        if (wa->cond_field) {
            fprintf( source, "  if ( self->%s & %d ) {\n", wa->cond_field, wa->cond_bits );
            indent++;
        }
        if (wa->is_list) {
            /* make Vector */
            fprintf( source, "%slen += wain_int_length(%d);\n", indent_val[indent], VECTOR_ID);
            fprintf( source, "%slen += wain_int_length(g_list_length(self->%s));\n", indent_val[indent], wa->name);
            fprintf( source, "%sGList *l_%s;\n", indent_val[indent], wa->name );
            fprintf( source, "%1$sfor(l_%2$s=self->%2$s; l_%2$s; l_%2$s = l_%2$s->next) {\n",
                    indent_val[indent], wa->name );
            indent++;
            mk_list_item( source, indent_val[indent], wa->klass, wa->type, wa->name);
            mk_list_item_len( source, indent_val[indent], "len", wa->klass, wa->name);
            indent--;
            fprintf( source, "%s}\n", indent_val[indent] );
        }
        else {
            mk_param_len( source, indent_val[indent], "len", wa->klass, wa->name );
        }
        if (wa->cond_field)
            fprintf( source, "  }\n" );
    }
    fprintf( source, "  return len;\n" );
    fprintf( source, "}\n" );
    fprintf( source, "\n" );
    
    /* === serialize === */

    fprintf( source, "void wain_%s_serialize( Wain%s *self, gchar *bytes )\n", 
            method_prefix, class_name );
    fprintf( source, "{\n" );
    fprintf( source, "  g_return_if_fail( WAIN_IS_%s(self) );\n", macro_name );
    fprintf( source, "  WainObjectClass *woc = WAIN_OBJECT_GET_CLASS(self);\n" );
    fprintf( source, "  wain_int_serialize( woc->id, bytes );\n  bytes += wain_int_length( woc->id );\n" );

    for (al = arglist; al; al = al->next) {
        WainArg *wa = al->data;
        indent = 1;
        if (wa->cond_field) {
            fprintf( source, "  if ( self->%s & %d ) {\n", wa->cond_field, wa->cond_bits );
            indent++;
        }
        if (wa->is_list) {
            /* make Vector */
            fprintf( source, "%1$swain_int_serialize( %2$d, bytes );\n%1$sbytes += wain_int_length( %2$d );\n",
                    indent_val[indent], VECTOR_ID );
            fprintf( source, "%swain_int_serialize( g_list_length( self->%s ), bytes );\n",
                    indent_val[indent], wa->name );
            fprintf( source, "%sbytes += wain_int_length( g_list_length( self->%s ) );\n",
                    indent_val[indent], wa->name );
            fprintf( source, "%sGList *l_%s;\n", indent_val[indent], wa->name );
            fprintf( source, "%1$sfor(l_%2$s=self->%2$s; l_%2$s; l_%2$s = l_%2$s->next) {\n",
                    indent_val[indent], wa->name );
            indent++;
            mk_list_item( source, indent_val[indent], wa->klass, wa->type, wa->name );
            mk_list_item_ser( source, indent_val[indent], "bytes", wa->klass, wa->name );
            mk_list_item_len( source, indent_val[indent], "bytes", wa->klass, wa->name );
            indent--;
            fprintf( source, "%s}\n", indent_val[indent] );
        }
        else {
            mk_param_ser( source, indent_val[indent], "bytes", wa->klass, wa->name );
            mk_param_len( source, indent_val[indent], "bytes", wa->klass, wa->name );
        }
        if (wa->cond_field)
            fprintf( source, "  }\n" );
    }

    fprintf( source, "}\n\n" );

    /* === from_bytes === */
    fprintf( source, "Wain%s *wain_%s_from_bytes( gchar *bytes ) { return g_object_new(WAIN_TYPE_%s, NULL); }\n\n",
            class_name, method_prefix, macro_name );

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
    constructors = g_hash_table_new( g_direct_hash, g_direct_equal );

    yyparse();

    if (headers && sources){
        FILE *src = fopen( "auto.h", "w" );

        fprintf( src, "#ifndef __WAIN_AUTO_H__\n" );
        fprintf( src, "#define __WAIN_AUTO_H__\n\n" );
        fprintf( src, "#include <glib-object.h>\n" );
        fprintf( src, "#include <glib.h>\n\n" );
        fprintf( src, "#include \"object.h\"\n\n" );
        GList *k, *keys = g_hash_table_get_keys( headers );
        for ( k = keys; k; k = k->next ){
            fprintf( src, "#include \"%s\"\n", (gchar *)k->data );
        }
        g_list_free(keys);
        fprintf( src, "\n#endif\n" );
        fclose(src);

        src = fopen( "auto.c", "w" );
        fprintf( src, "#include \"auto.h\"\n\n" );
        
        keys = g_hash_table_get_keys( sources );
        for ( k = keys; k; k = k->next ){
            fprintf( src, "#include \"%s\"\n", (gchar *)k->data );
        }

        fprintf( src, "\nWainObject *wain_object_from_bytes( gchar *bytes )\n{\n" );
        fprintf( src, "  guint32 id = *(guint32 *)bytes;\n" );
        fprintf( src, "  bytes += 4;\n" );
        fprintf( src, "  switch (id) {\n" );

        GHashTableIter i;
        g_hash_table_iter_init( &i, constructors );
        gpointer hash, method;
        while ( g_hash_table_iter_next( &i, &hash, &method ) ) {
            fprintf( src, "  case 0x%x:\n", GPOINTER_TO_INT(hash) );
            fprintf( src, "    return wain_%s_from_bytes(bytes);\n    break;\n", method ); 
        }
        fprintf( src, "  default:\n" );
        fprintf( src, "    return NULL;\n" );
        fprintf( src, "  }\n}\n" );

        fclose(src);
    }

    return 0;
}

