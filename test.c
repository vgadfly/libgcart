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
    printf ("one is %s\n", wain_object_tl_name(WAIN_OBJECT(one)) );
    one->i = 0x33;
    one->s = "lolkek";
    one->d = 0.1;
    one->l = -1;
    one->b = TRUE;
    
    len =  wain_object_length( WAIN_OBJECT(one) );
    printf( "One serialized len is %u\n", len );
    bytes = g_malloc(len);
    wain_object_serialize( WAIN_OBJECT(one), bytes );
    
    for ( i = 0; i < len; i+=4 )
        printf("%08X.", *(guint32*)(bytes+i));
    printf("\n");

    g_free(bytes);

    WainTwo *two = g_object_new( WAIN_TYPE_TWO, NULL );
    printf ("two is %s\n", wain_object_tl_name(WAIN_OBJECT(two)) );
    two->t1 = one;
    two->t2 = one;
    len =  wain_object_length( WAIN_OBJECT(two) );
    printf( "Two serialized len is %u\n", len );
    bytes = g_malloc(len);
    wain_object_serialize( WAIN_OBJECT(two), bytes );
    
    for ( i = 0; i < len; i+=4 )
        printf("%08X.", *(guint32*)(bytes+i));
    printf("\n");
    
    g_free(bytes);

    WainThree *three = g_object_new( WAIN_TYPE_THREE, NULL );
    printf ("three is %s\n", wain_object_tl_name(WAIN_OBJECT(three)) );
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
    
    len = wain_object_length( WAIN_OBJECT(three) );
    printf("Three serialized len is %d\n", len);
    bytes = g_malloc(len);
    wain_object_serialize( WAIN_OBJECT(three), bytes );
    
    for ( i = 0; i < len; i+=4 )
        printf("%08X.", *(guint32*)(bytes+i));
    printf("\n");
    
    g_free(bytes);

    WainTestFour *four = g_object_new( WAIN_TYPE_TEST_FOUR, NULL );
    printf ("four is %s\n", wain_object_tl_name(WAIN_OBJECT(four)) );

    return 0;
}
