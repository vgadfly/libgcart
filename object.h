#ifndef __WAIN_OBJECT_H__
#define __WAIN_OBJECT_H__

#include <glib-object.h>
#include <glib.h>

#include "stream.h"

G_BEGIN_DECLS

#define WAIN_TYPE_OBJECT                   wain_object_get_type()
G_DECLARE_DERIVABLE_TYPE (WainObject, wain_object, WAIN, OBJECT, GObject)

struct _WainObjectClass
{
    GObjectClass parent_class;

    /* class members */
    guint32 id;

    /* virtual methods */
    void (*serialize)( WainObject *, WainStream * );
    WainObject *(*from_bytes)( WainStream * );
    
    gpointer padding[8];
};

void wain_object_serialize( WainObject *, WainStream * );

/* primitive type ser/des */
void wain_str_serialize( gchar *, WainStream * );
gchar *wain_str_from_bytes( WainStream * );

void wain_int_serialize( gint32, WainStream * );
gint32 wain_int_from_bytes( WainStream * );

void wain_long_serialize( gint64, WainStream * );
gint64 wain_long_from_bytes( WainStream * );

#endif /* __WAIN_OBJECT_H__ */

