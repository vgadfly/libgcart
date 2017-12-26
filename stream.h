#ifndef __WAIN_STREAM_H__
#define __WAIN_STREAM_H__

#include <glib.h>

typedef struct _wain_stream wain_stream;

struct _wain_stream {
    guint32 (*read_word)( void* );
    void (*write_word)( void *, guint32 );
};

#endif /* __WAIN_STREAM_H__ */

