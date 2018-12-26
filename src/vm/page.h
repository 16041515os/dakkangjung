#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>
#include <stdbool.h>
#include "filesys/off_t.h"

struct thread;
struct file;
typedef struct hash *supt_t;

supt_t supt_create(void);
struct supte *supte_lookup_page(supt_t supt, void *upage);
bool supt_install_swap_page(supt_t supt, void *upage, uint32_t swap_idx, bool writable);

void *supt_load_page(struct thread *thread, void *upage);
bool supt_install_zero_page(supt_t supt, void *upage);
bool supt_install_file(supt_t, void *upage, struct file *file, off_t, uint32_t, uint32_t, bool);
void supt_destroy(supt_t);

#endif
