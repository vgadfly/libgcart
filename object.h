#ifndef __WAIN_OBJECT_H__
#define __WAIN_OBJECT_H__

#include <glib-gobject.h>
#include <glib.h>

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
    void serialize(void);

    void reserved1(void);
    void reserved2(void);
    void reserved3(void);
};

struct _WainObjectClass
{
    GObjectClass parent_class;

    /* class members */
    WainObject *read_object(guint32);
    
    void reserved1(void);
    void reserved2(void);
    void reserved3(void);
};

#endif /* __WAIN_OBJECT_H__ */

