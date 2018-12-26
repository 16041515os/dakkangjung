#ifndef SWAP_H
#define SWAP_H

#include <stdint.h>

void swap_init(void);
uint32_t swap_out(void *kpage);
void swap_in(uint32_t swap_idx, void *kpage);
void swap_slot_free(uint32_t swap_idx);

#endif
