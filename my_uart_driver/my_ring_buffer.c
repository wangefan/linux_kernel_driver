#include "my_ring_buffer.h"

void init_buffer(struct my_ring_buffer *buf) {
  buf->head = 0;
  buf->tail = 0;
}

//   Buffer status(BUF_LEN = 8)
//   O = data
//   x = empty
//
//   index:   0   1   2   3   4   5   6   7
//            x   x   x   O   O   O   O   O
//                        ↑
//                        tail = 3
//                    ↑
//                    head = 2
//
//   if write to head, NEXT_PLACE(head) = (2 + 1) & 0x7 = 3
//   would reach tail ⇒ buffer is full
unsigned int is_full(struct my_ring_buffer *buf) {
  return NEXT_PLACE(buf->head) == buf->tail;
}

//   Buffer status(BUF_LEN = 8)
//   x = empty
//
//   index:   0   1   2   3   4   5   6   7
//            x   x   x   x   x   x   x   x
//                    ↑
//                    tail = 2
//                    ↑
//                    head = 2
//
//   head == tail means nothing to read, the buffer is empty.
unsigned int is_empty(struct my_ring_buffer *buf) {
  return buf->head == buf->tail;
}

void put_data(struct my_ring_buffer *buf, unsigned char data) {
  if (is_full(buf))
    return;
  buf->buffer[buf->head] = data;
  buf->head = NEXT_PLACE(buf->head);
}

unsigned char get_data(struct my_ring_buffer *buf) {
  unsigned char data = 0;
  if (is_empty(buf))
    return data;
  data = buf->buffer[buf->tail];
  buf->tail = NEXT_PLACE(buf->tail);
  return data;
}

//   Buffer status(BUF_LEN = 8)
//   case1: head >= tail
//
//   index:   0   1   2   3   4   5   6   7
//            x   x   O   x   x   x   x   x
//                    ↑
//                    tail = 2
//                        ↑
//                        head = 3
//   count = head - tail = 3 - 2 = 1
//
//   case2: head < tail
//
//   index:   0   1   2   3   4   5   6   7
//            O   x   O   O   O   O   O   O
//                    ↑
//                    tail = 2
//                ↑
//                head = 1
//   count = BUF_LEN - (tail - head) = 8 - (2 - 1) = 7
unsigned int get_data_count(struct my_ring_buffer *buf) {
  if (buf->head >= buf->tail)
    return buf->head - buf->tail;
  else
    return BUF_LEN - (buf->tail - buf->head);
}

//   Buffer status(BUF_LEN = 8)
//   note: the max space is BUF_LEN - 1
//   case1: head >= tail
//
//   index:   0   1   2   3   4   5   6   7
//            x   x   O   x   x   x   x   x
//                    ↑
//                    tail = 2
//                        ↑
//                        head = 3
//   count = head - tail = 3 - 2 = 1
//   space = (BUF_LEN - 1 ) - count = 7 - 1 = 6
//
//   case2: head < tail
//
//   index:   0   1   2   3   4   5   6   7
//            O   x   O   O   O   O   O   O
//                    ↑
//                    tail = 2
//                ↑
//                head = 1
//   count = BUF_LEN - (tail - head) = 8 - (2 - 1) = 7
//   space = (BUF_LEN - 1) - count = 7 - 7 = 0
unsigned int get_space_count(struct my_ring_buffer *buf) {
  return (BUF_LEN - 1) - get_data_count(buf);
}