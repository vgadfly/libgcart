#include <stdio.h>
#include <stdlib.h>

#include <glib.h>

#include "object.h"
#include "auto.h"
#include "dummy.h"

int main( int argc, char *argv[] )
{
    WainDummyStream *ds = wain_dummy_stream_instance();
    WainOne *one = g_object_new( WAIN_TYPE_ONE, NULL );
    one->i = 0x33;
    one->s = "lolkek";
    printf( "== One ==\n" );
    wain_object_serialize( one, ds );

    WainTwo *two = g_object_new( WAIN_TYPE_TWO, NULL );
    two->t1 = one;
    two->t2 = one;
    printf( "== Two ==\n" );
    wain_object_serialize( two, ds );

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
    printf( "== Three ==\n" );
    wain_object_serialize( three, ds );

    return 0;
}
