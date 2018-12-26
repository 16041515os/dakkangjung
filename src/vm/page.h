#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>
#include <stdbool.h>
#include <hash.h>
#include "filesys/off_t.h"

enum supte_tag {
  P_ZERO,
  P_SWAP,
  P_FILE
};

union supte_data {
  struct {
    uint32_t swap_idx;
  } swap;

  struct {
    struct file *file;
    off_t ofs;
    uint32_t read_bytes;
    uint32_t zero_bytes;
  } file;
};

struct supte {
  // hash key
  void *upage;

  // hash value
  enum supte_tag tag;
  union supte_data data;
  bool writable;

  struct hash_elem hash_elem;
};

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
