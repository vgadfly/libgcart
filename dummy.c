#include "dummy.h"

#include <stdio.h>

static guint32 wain_read_word( void *dummy )
{
    char buf[32] = {0};
    printf("enter word: ");
    fgets( buf, sizeof(buf), stdin );

    return strtoul(buf, NULL, 0);
}

static void wain_write_word( void *dummy, guint32 word )
{
    printf("output: 0x%08x\n", word);
}

static WainDummyStream dummy_instance = {
    .read_word = wain_read_word,
    .write_word = wain_write_word
};

WainDummyStream *wain_dummy_stream_instance()
{
    return &dummy_instance;
}

