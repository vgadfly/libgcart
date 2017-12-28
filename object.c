#include "object.h"

G_DEFINE_TYPE(WainObject, wain_object, G_TYPE_OBJECT);

static void wain_object_class_init( WainObjectClass *klass )
{
    klass->serialize = NULL;
}

static void wain_object_init( WainObject *obj )
{
}

GObject *wain_object_new(void)
{
    return g_object_new( WAIN_TYPE_OBJECT, 0 );
}

void wain_int_serialize( gint32 i, WainStream *stream )
{
    stream->write_word( stream, htole32(i) );
}

void wain_long_serialize( gint64 l, WainStream *stream )
{
    stream->write_word( stream, htole32(l&0xFFFFFFFF) );
    stream->write_word( stream, htole32((l>>32)&0xFFFFFFFF) );
}

void wain_str_serialize( gchar *s, WainStream *stream )
{
    ;
}

void wain_object_serialize( WainObject *obj, WainStream *stream )
{
    ;
}
