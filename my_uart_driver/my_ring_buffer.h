#ifndef MY_RING_BUFFER_H
#define MY_RING_BUFFER_H

#define BUF_LEN 1024
#define NEXT_PLACE(idx) ((idx + 1) % BUF_LEN)

struct my_ring_buffer {
  unsigned char buffer[BUF_LEN];
  unsigned int head; // where to write next
  unsigned int tail; // where to read next
};
void init_buffer(struct my_ring_buffer *buf);
unsigned int is_full(struct my_ring_buffer *buf);
void put_data(struct my_ring_buffer *buf, unsigned char data);
unsigned char get_data(struct my_ring_buffer *buf);
unsigned int get_data_count(struct my_ring_buffer *buf);
unsigned int get_space_count(struct my_ring_buffer *buf);

#endif // MY_RING_BUFFER_H