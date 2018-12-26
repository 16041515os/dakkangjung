#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct hash *supt_t;

supt_t supt_create(void);
struct supte *supte_lookup_page(supt_t supt, void *upage);
bool supt_load_page(supt_t supt, uint32_t *pd, void *upage);
bool supt_install_zero_page(supt_t supt, void *upage);
void supt_destroy(supt_t);

#endif
