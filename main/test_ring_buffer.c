// Testing ring buffers

#include <stdio.h>
#include <stdint.h>

typedef struct {
    uint8_t * const buffer; // constant pointer to array of bytes
    int head;
    int tail;
    const int maxlen;
} circ_bbuf_t;

uint8_t my_queue_data_space[16]; // 16 byte queue

circ_bbuf_t my_queue = {
    .buffer = my_queue_data_space,
    .head = 0,
    .tail = 0,
    .maxlen = 16
};

int circ_bbuf_push(circ_bbuf_t *c, uint8_t data) {
    int next;
    
    next = c->head+1; // head after write
    if (next >= c->maxlen) {
        next = 0;
    }
    if (next == c->tail) { // if head + 1 == tail, buffer full
        return -1;
    }
    
    c->buffer[c->head] = data; // load data then move
    c->head = next;   // head to next data offset
    return 0; // return success
    
}

int circ_bbuf_pop(circ_bbuf_t *c, uint8_t *data) {
    int next;
    
    if (c->head == c->tail) { // no data in this case
        return -1;
    }
    next = c->tail + 1; // tail after read
    if (next >= c->maxlen) {
        next = 0;
    }
    *data = c->buffer[c->tail]; // read data
    c->tail = next; // tail to next offset
    
    return 0;
}

int main()
{
    printf("Hello World\n");
    
    uint8_t out_data=0;
    uint8_t in_data = 0x55;
    
    if (circ_bbuf_push(&my_queue, in_data)) {
        printf("Out of space in CB\n");
        return -1;
    }
    
    printf("Head Buffer: %d\n", my_queue.head);
    printf("Tail Buffer: %d\n", my_queue.tail);
    printf("Current Buffer Value: 0x%x\n", my_queue.buffer[0]);

    
    if (circ_bbuf_pop(&my_queue, &out_data)) {
        printf("CB is empty \n");
        return -1;
    }
    
    printf("Head Buffer: %d\n", my_queue.head);
    printf("Tail Buffer: %d\n", my_queue.tail);
    printf("Current Buffer Value: 0x%x\n", my_queue.buffer[0]);

    
    printf("Push: 0x%x\n", in_data);
    printf("Pop: 0x%x\n", out_data);
    
    return 0;
}