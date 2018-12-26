#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>
#include <stdbool.h>

struct thread;
typedef struct hash *supt_t;

supt_t supt_create(void);
struct supte *supte_lookup_page(supt_t supt, void *upage);
bool supt_set_swap_page(supt_t supt, void *upage, uint32_t swap_idx, bool writable);

void *supt_load_page(struct thread *thread, void *upage);
bool supt_install_zero_page(supt_t supt, void *upage);
void supt_destroy(supt_t);

#endif
