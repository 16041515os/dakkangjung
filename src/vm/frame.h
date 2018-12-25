#ifndef FRAME_H
#define FRAME_H

#include "threads/palloc.h"

void frame_init(void);
void *frame_alloc(enum palloc_flags);

void frame_set_page(uint32_t *pd, void *upage, void *kpage);
void frame_entry_free(uint32_t *pd, void *);

void frame_set_pinned(void *, bool);

#endif
