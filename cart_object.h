#ifndef __CART_OBJECT_H__
#define __CART_OBJECT_H__

#include <glib-gobject.h>
#include <glib.h>

G_BEGIN_DECLS

#define CART_TYPE_OBJECT                   cart_object_get_type()
#define CART_OBJECT(obj)                   (G_TYPE_CHECK_INSTANCE_CAST ((obj), CART_TYPE_OBJECT, CartObject))
#define CART_IS_OBJECT(obj)                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CART_TYPE_OBJECT))
#define CART_OBJECT_CLASS(_class)          (G_TYPE_CHECK_CLASS_CAST ((_class), CART_TYPE_OBJECT, CartObjectClass))
#define CART_IS_OBJECT_CLASS(_class)       (G_TYPE_CHECK_CLASS_TYPE ((_class), CART_TYPE_OBJECT))
#define CART_OBJECT_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), CART_TYPE_OBJECT, CartObjectClass))

typedef struct _CartObject CartObject;
typedef struct _CartObjectClass CartObjectClass;

struct _CartObject
{
    GObject parent_instance;

    /* instance members */
    void serialize(void);

    void reserved1(void);
    void reserved2(void);
    void reserved3(void);
};

struct _CartObjectClass
{
    GObjectClass parent_class;

    /* class members */
    CartObject *read_object(guint32);
    
    void reserved1(void);
    void reserved2(void);
    void reserved3(void);
};

#endif /* __CART_OBJECT_H__ */

