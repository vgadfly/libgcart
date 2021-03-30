#ifndef __WAIN_OBJECT_H__
#define __WAIN_OBJECT_H__

#include <glib-object.h>
#include <glib.h>

G_BEGIN_DECLS

#define WAIN_TYPE_OBJECT                   wain_object_get_type()

/** Base class for TL objects */
G_DECLARE_DERIVABLE_TYPE (WainObject, wain_object, WAIN, OBJECT, GObject)

struct _WainObjectClass
{
    GObjectClass parent_class;

    /** TL constructor hash id */
    guint32 id;

    /** TL type name */
    const gchar *tl_name;

    /** serialized length of the object */
    gint32 (*length)( WainObject * );

    /** serialize object */
    void (*serialize)( WainObject *, gchar * );

    gpointer reserved[8];
};

const gchar *wain_object_tl_name( WainObject * );
gint32 wain_object_length( WainObject * );
void wain_object_serialize( WainObject *, gchar * );
WainObject *wain_object_from_bytes( gchar * );

/* === primitive type ser/des === */

gint32 wain_str_length( gchar * );
void wain_str_serialize( gchar *, gchar * );
gchar *wain_str_from_bytes( gchar * );

gint32 wain_bytes_length( gchar * );
void wain_bytes_serialize( gchar *, gchar * );
gchar *wain_bytes_from_bytes( gchar * );

static inline gint32 wain_int_length( gint32 i ) { return 4; }
void wain_int_serialize( gint32, gchar * );
gint32 wain_int_from_bytes( gchar * );

static inline gint32 wain_nat_length( guint32 u ) { return 4; }
void wain_nat_serialize( guint32, gchar * );
guint32 wain_nat_from_bytes( gchar * );

static inline gint32 wain_long_length( gint64 l ) { return 8; }
void wain_long_serialize( gint64, gchar * );
gint64 wain_long_from_bytes( gchar * );

static inline gint32 wain_int128_length( gint32 * i128 ) { return 16; }
void wain_int128_serialize( gint32 *, gchar * );
gint32 *wain_int128_from_bytes( gchar * );

static inline gint32 wain_int256_length( gint32 * i256 ) { return 32; }
void wain_int256_serialize( gint32 *, gchar * );
gint32 *wain_int256_from_bytes( gchar * );

static inline gint32 wain_double_length( gdouble d ) { return 8; }
void wain_double_serialize( gdouble, gchar * );
gdouble wain_double_from_bytes( gchar * );

static inline gint32 wain_bool_length( gboolean b ) { return 4; }
void wain_bool_serialize( gboolean, gchar * );
gboolean wain_bool_from_bytes( gchar * );

G_END_DECLS

#endif /* __WAIN_OBJECT_H__ */

