#include "object.h"

#include <stdlib.h>
#include <string.h>

#ifndef ALIGN
#define ALIGN(x,a) __ALIGN_MASK(x,(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask) (((x)+(mask))&~(mask))
#endif

#define BOOL_TRUE_ID  0x997275b5
#define BOOL_FALSE_ID 0xbc799737

G_DEFINE_TYPE(WainObject, wain_object, G_TYPE_OBJECT);

/* === Object === */
static void wain_object_class_init( WainObjectClass *klass )
{
    klass->serialize = NULL;
    klass->length = NULL;
    klass->from_bytes = NULL;
    klass->tl_name = NULL;
}

static void wain_object_init( WainObject *obj )
{
}

GObject *wain_object_new(void)
{
    return g_object_new( WAIN_TYPE_OBJECT, 0 );
}

const gchar *wain_object_tl_name( WainObject *obj )
{
    WainObjectClass *woc = WAIN_OBJECT_GET_CLASS(obj);
    return woc->tl_name; 
}

gint32 wain_object_length( WainObject *obj )
{
    WainObjectClass *woc = WAIN_OBJECT_GET_CLASS(obj);
    /* call virtual length */
    return woc->length(obj);
}

void wain_object_serialize( WainObject *obj, gchar *bytes )
{
    WainObjectClass *woc = WAIN_OBJECT_GET_CLASS(obj);
    /* call virtual serialize */
    woc->serialize( obj, bytes );
}

/* === int === */
void wain_int_serialize( gint32 i, gchar *bytes )
{
    *(guint32 *)bytes = htole32(i);
}

gint32 wain_int_from_bytes( gchar *bytes )
{
    return le32toh(*(guint32 *)bytes);
}

/* === nat === */
void wain_nat_serialize( guint32 u, gchar *bytes )
{
    *(guint32 *)bytes = htole32(u);
}

guint32 wain_nat_from_bytes( gchar *bytes )
{
    return le32toh(*(guint32 *)bytes);
}

/* === long === */
void wain_long_serialize( gint64 l, gchar *bytes )
{
    *(guint64 *)bytes = htole64(l);
}

gint64 wain_long_from_bytes( gchar *bytes )
{
    return le64toh(*(guint64 *)bytes);
}

/* === double === */
void wain_double_serialize( gdouble d, gchar *bytes )
{
    *(gdouble *)bytes = d;
}

gdouble wain_double_from_bytes( gchar *bytes )
{
    return (*(gdouble *)bytes);
}

/* === bool === */
void wain_bool_serialize( gboolean b, gchar *bytes )
{
    *(gint32 *)bytes = b ? BOOL_TRUE_ID : BOOL_FALSE_ID;
}

gboolean wain_bool_from_bytes( gchar *bytes )
{
    gint32 id = *(gint32 *)bytes;
    return (id == BOOL_TRUE_ID);
}

/* === str === */
gint32 wain_str_length( gchar *s )
{
    int len = strlen(s);
    if (len < 254) {
        return ALIGN(len+1, 4);
    }
    else {
        return ALIGN(len, 4) + 4;
    }
}

void wain_str_serialize( gchar *s, gchar *bytes )
{
    const guint32 mask[] = {0, 0xff, 0xffff, 0xffffff};
    guint32 len = strlen(s);
    int i;

    if ( len < 254 ) {
        guint32 val = len & 0xFF | ((guint32)s[0] << 8) | ((guint32)s[1] << 16) | ((guint32)s[2] << 24);
        if (len < 3)
            val &= htole32( mask[len+1] );
        *(guint32 *)bytes = val;
        bytes += 4;
        
        for ( i = 3; i < len; i += 4, bytes += 4 ) {
            val = (guint32)s[i] | ((guint32)s[i+1] << 8) | ((guint32)s[i+2] << 16) | ((guint32)s[i+3] << 24);
            if (len-i < 4)
                val &= htole32( mask[len-i] );
            *(guint32 *)bytes = val;
        }
    }
    else
    {
        guint32 val = 254 | (len << 8);
        *(guint32 *)bytes = val;
        bytes += 4;

        for ( i = 0; i < len; i += 4, bytes += 4 ) {
            val = (guint32)s[i] | ((guint32)s[i+1] << 8) | ((guint32)s[i+2] << 16) | ((guint32)s[i+3] << 24);
            if (len-i < 4)
                val &= htole32( mask[len-i] );
            *(guint32 *)bytes = val;
        }
    }
}

gchar *wain_str_from_bytes( gchar *bytes )
{
    return NULL;
}

