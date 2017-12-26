#include "dummy.h"

#include <stdio.h>

static guint32 wain_read_word( void *dummy )
{
    char buf[32] = {0};
    printf("enter word: ");
    fgets( buf, sizeof(buf), stdin );

    return strtoul(buf, NULL, 0);
}

static void wain_write_word( void *dummy, guin32 word )
{
    printf("output: 0x%08x", word);
}

static wain_dummy_stream {
    .read_word = wain_read_word,
    .write_word = wain_write_word
} dummy_instance;

wain_dummy_stream *wain_dummy_stream_instance()
{
    return &dummy_instance;
}

