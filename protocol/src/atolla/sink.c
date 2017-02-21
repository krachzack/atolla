#include "atolla/sink.h"

#include "test/assert.h"

AtollaSink atolla_sink_make(int port)
{
    AtollaSink sink;
    assert(false);
    return sink;
}

AtollaSinkState atolla_sink_state(AtollaSink* sink)
{
    return sink->state;
}

bool atolla_sink_get(AtollaSink* sink, void* frame, size_t frame_len)
{
    assert(false);
    return false;
}
