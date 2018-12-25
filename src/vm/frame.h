#ifndef FRAME_H
#define FRAME_H

#include "threads/palloc.h"

void frame_init(void);
void *frame_alloc(enum palloc_flags, void *upage);

void frame_dealloc(void *);
void frame_free(void *);

void frame_set_pinned(void *, bool);

#endif
