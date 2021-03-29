#include <stdio.h>
#include <stdlib.h>

#include <glib.h>

#include "object.h"
#include "auto.h"
#include "dummy.h"

int main( int argc, char *argv[] )
{
    gchar *bytes;
    guint len;
    int i;

    WainOne *one = g_object_new( WAIN_TYPE_ONE, NULL );
    one->i = 0x33;
    one->s = "lolkek";
    one->d = 0.1;
    one->l = -1;
    one->b = TRUE;
    
    len =  wain_object_length( one );
    printf( "One serialized len is %u\n", len );
    bytes = g_malloc(len);
    wain_object_serialize( one, bytes );
    
    for ( i = 0; i < len; i+=4 )
        printf("%08X.", *(guint32*)(bytes+i));
    printf("\n");

    g_free(bytes);

    WainTwo *two = g_object_new( WAIN_TYPE_TWO, NULL );
    two->t1 = one;
    two->t2 = one;
    len =  wain_object_length( two );
    printf( "Two serialized len is %u\n", len );
    bytes = g_malloc(len);
    wain_object_serialize( two, bytes );
    
    for ( i = 0; i < len; i+=4 )
        printf("%08X.", *(guint32*)(bytes+i));
    printf("\n");
    
    g_free(bytes);

    WainThree *three = g_object_new( WAIN_TYPE_THREE, NULL );
    GList *list = NULL;
    gint32 *pi = g_new(gint32, 1);
    *pi = 1;
    list = g_list_append( list, pi );
    pi = g_new(gint32, 1);
    *pi = 2;
    list = g_list_append( list, pi );
    pi = g_new(gint32, 1);
    *pi = 3;
    list = g_list_append( list, pi );
    three->vi = list;

    list = g_list_append( NULL, two );
    three->vo = list;

    list = g_list_append( NULL, "s_one" );
    list = g_list_append( list, "s_two" );
    three->vs = list;
    
    len = wain_object_length( three );
    printf("Three serialized len is %d\n", len);
    bytes = g_malloc(len);
    wain_object_serialize( three, bytes );
    
    for ( i = 0; i < len; i+=4 )
        printf("%08X.", *(guint32*)(bytes+i));
    printf("\n");
    
    g_free(bytes);

    WainFour *four = g_object_new( WAIN_TYPE_FOUR, NULL );


    return 0;
}
