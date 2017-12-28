#include "object.h"

#include <stdlib.h>
#include <string.h>

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
    const guint32 mask[] = {0, 0xff, 0xffff, 0xffffff};
    int len = strlen(s);
    if (len < 254) {
        guint32 val = len | (s[0] << 8) | (s[1] << 16 | s[2] << 24);
        if (len < 3)
            val &= mask[len+1];
        wain_int_serialize( val, stream );
        int i;
        for (i = 3; i < len; i+=4) {
            val = s[i] | (s[i+1] << 8) | (s[i+2] << 16 | s[i+3] << 24);
            if (len-i < 4)
                val &= mask[len-i];
            wain_int_serialize( val, stream );
        }
    }
}

void wain_object_serialize( WainObject *obj, WainStream *stream )
{
    WainObjectClass *woc = WAIN_OBJECT_GET_CLASS(obj);
    woc->serialize( obj, stream );
}

