#include "wain_object.h"

G_DEFINE_TYPE(WainObject, wain_object, G_TYPE_OBJECT);

static void wain_object_class_init( WainObjectClass *klass )
{
}

static void wain_object_init( WainObject *obj )
{
}

GObject *wain_object_new(void)
{
    return g_object_new( WAIN_TYPE_OBJECT, 0 );
}

