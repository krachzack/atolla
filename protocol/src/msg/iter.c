#include "test/assert.h"
#include "msg/iter.h"
#include "mem/uint16le.h"

static MemBlock msg_iter_payload(MsgIter* iter);
static uint16_t msg_iter_payload_length(MsgIter* iter);

MsgIter msg_iter_make(
    void* msg_buffer,
    size_t msg_buffer_size
)
{
    MsgIter iter;

    iter.msg_buf_start = msg_buffer;
    iter.msg_buf_end = iter.msg_buf_start + msg_buffer_size;

    return iter;
}

bool msg_iter_has_msg(MsgIter* iter)
{
    return iter->msg_buf_start < iter->msg_buf_end;
}

void msg_iter_next(MsgIter* iter)
{
    assert(msg_iter_has_msg(iter));

    size_t msg_len = 5 + msg_iter_payload_length(iter);
    iter->msg_buf_start += msg_len;
}

MsgType msg_iter_type(MsgIter* iter)
{
    assert(msg_iter_has_msg(iter));

    uint8_t msg_type_byte = iter->msg_buf_start[0];
    assert((msg_type_byte >= 0 && msg_type_byte <= 2) || msg_type_byte == 255);
    return (MsgType) msg_type_byte;
}

uint16_t msg_iter_msg_id(MsgIter* iter)
{
    assert(msg_iter_has_msg(iter));

    uint16_t* msg_id_ptr = (uint16_t*) (iter->msg_buf_start + 1);
    return mem_uint16le_from(*msg_id_ptr);
}

static uint16_t msg_iter_payload_length(MsgIter* iter)
{
    assert(msg_iter_has_msg(iter));

    uint16_t* payload_len_ptr = (uint16_t*) (iter->msg_buf_start + 3);
    return mem_uint16le_from(*payload_len_ptr);
}

static MemBlock msg_iter_payload(MsgIter* iter)
{
    assert(msg_iter_has_msg(iter));

    uint16_t* payload_len_ptr = (uint16_t*) (iter->msg_buf_start + 3);

    uint16_t payload_len = mem_uint16le_from(*payload_len_ptr);
    void* payload = payload_len_ptr + 1;

    return mem_block_make(payload, payload_len);
}

uint8_t msg_iter_borrow_frame_length(MsgIter* iter)
{
    assert(msg_iter_type(iter) == MSG_TYPE_BORROW);
    MemBlock payload = msg_iter_payload(iter);
    return ((uint8_t*) payload.data)[0];
}

uint8_t msg_iter_borrow_buffer_length(MsgIter* iter)
{
    assert(msg_iter_type(iter) == MSG_TYPE_BORROW);
    MemBlock payload = msg_iter_payload(iter);
    return ((uint8_t*) payload.data)[1];
}

uint8_t msg_iter_enqueue_frame_idx(MsgIter* iter)
{
    assert(msg_iter_type(iter) == MSG_TYPE_ENQUEUE);
    MemBlock payload = msg_iter_payload(iter);
    return ((uint8_t*) payload.data)[0];
}

MemBlock msg_iter_enqueue_frame(MsgIter* iter)
{
    assert(msg_iter_type(iter) == MSG_TYPE_ENQUEUE);
    MemBlock payload = msg_iter_payload(iter);
    return mem_block_slice(&payload, 3, payload.size-3);
}

uint16_t msg_iter_fail_offending_msg_id(MsgIter* iter)
{
    assert(msg_iter_type(iter) == MSG_TYPE_FAIL);
    MemBlock payload = msg_iter_payload(iter);
    uint16_t* offending_msg_id_ptr = (uint16_t*) payload.data;
    return *offending_msg_id_ptr;
}

uint8_t msg_iter_fail_error_code(MsgIter* iter)
{
    assert(msg_iter_type(iter) == MSG_TYPE_FAIL);
    MemBlock payload = msg_iter_payload(iter);
    uint16_t* offending_msg_id_ptr = (uint16_t*) payload.data;
    uint8_t* error_code_ptr = (uint8_t*) (offending_msg_id_ptr + 1);
    return *error_code_ptr;
}
