#ifndef FRAME_H
#define FRAME_H

#include <stdint.h>
#include <stdbool.h>
#include "threads/palloc.h"

struct thread;

void frame_init(void);
void *frame_alloc(struct thread *, enum palloc_flags);

void frame_set_page(uint32_t *pd, void *upage, void *kpage, bool writable);
void frame_free_hard(uint32_t *pd, void *);
void frame_entry_free(uint32_t *pd, void *);

void frame_set_pinned(void *, bool);

#endif
