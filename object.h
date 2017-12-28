#ifndef __WAIN_OBJECT_H__
#define __WAIN_OBJECT_H__

#include <glib-object.h>
#include <glib.h>

#include "stream.h"

G_BEGIN_DECLS

#define WAIN_TYPE_OBJECT                   wain_object_get_type()
#define WAIN_OBJECT(obj)                   (G_TYPE_CHECK_INSTANCE_CAST ((obj), WAIN_TYPE_OBJECT, WainObject))
#define WAIN_IS_OBJECT(obj)                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), WAIN_TYPE_OBJECT))
#define WAIN_OBJECT_CLASS(_class)          (G_TYPE_CHECK_CLASS_CAST ((_class), WAIN_TYPE_OBJECT, WainObjectClass))
#define WAIN_IS_OBJECT_CLASS(_class)       (G_TYPE_CHECK_CLASS_TYPE ((_class), WAIN_TYPE_OBJECT))
#define WAIN_OBJECT_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), WAIN_TYPE_OBJECT, WainObjectClass))

typedef struct _WainObject WainObject;
typedef struct _WainObjectClass WainObjectClass;

struct _WainObject
{
    GObject parent_instance;

    /* instance members */
};

struct _WainObjectClass
{
    GObjectClass parent_class;

    /* class members */

    /* virtual methods */
    void (*serialize)( WainObject *, WainStream * );
    WainObject *(*from_bytes)( WainStream * );
    
    gpointer padding[8];
};

/* primitive type ser/des */
void wain_str_serialize( WainStream *, gchar * );
gchar *wain_str_from_bytes( WainStream * );

void wain_int_serialize( WainStream *, gint32 );
gint32 wain_int_from_bytes( WainStream * );

void wain_long_serialize( WainStream *, gint64 );
gint64 wain_long_from_bytes( WainStream * );

#endif /* __WAIN_OBJECT_H__ */

