#include "cart_object.h"

G_DEFINE_TYPE(CartObject, cart_object, G_TYPE_OBJECT);

static void cart_object_class_init( CartObjectClass *klass )
{
}

static void cart_object_init( CartObject *obj )
{
}

GObject *cart_object_new(void)
{
    return g_object_new( CART_TYPE_OBJECT, 0 );
}

