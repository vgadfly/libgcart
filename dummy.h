#ifndef __WAIN_DUMMY_H__
#define __WAIN_DUMMY_H__

#include <glib.h>

typedef struct _wain_dummy_stream wain_dummy_stream;

struct _wain_dummy_stream {
    guint32 read_word( void* );
    void write_word( void *, guint32 );
};

wain_dummy_stream *wain_dummy_stream_instance();

#endif /* __WAIN_DUMMY_H__
