
typedef struct _tl_list {
    void *data;
    struct _tl_list *next;
} tl_list;

typedef struct _tl_type {
    char *name;
    int modifiers;
    tl_list *t_args; 
} tl_type;

#define TYPE_MOD_NONE 0
#define TYPE_MOD_BANG 1
#define TYPE_MOD_TEMPLATE 2

typedef struct _tl_arg {
    char *name;
    tl_type *type;
    char *cond_field;
    int cond_bitmask;
    int modifiers;
} tl_arg;

#define ARG_MOD_NONE 0
#define ARG_MOD_COND 1
#define ARG_MOD_MULT 2

typedef struct _tl_cond {
    char *name;
    int bitmask;
} tl_cond;

int yylex(void);
int yyparse(void);
void yyerror( const char * );
void tl_type_gen( char *constr, int hash, tl_type *type, tl_list *args );
void tl_func_gen( char *name, int hash, tl_type *res, tl_list *args );

