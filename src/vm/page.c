#include "page.h"

#include <debug.h>
#include <hash.h>
#include <string.h>
#include "threads/malloc.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "filesys/file.h"
#include "frame.h"
#include "swap.h"

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

static unsigned _supte_hash_func(const struct hash_elem *e, void *aux);
static bool _supte_less_func(struct hash_elem const *a, struct hash_elem const *b, void *aux);
static void _supte_free_func(struct hash_elem *e, void *aux);

supt_t supt_create() {
  struct hash *supt = malloc(sizeof *supt);
  ASSERT(supt != NULL);
  hash_init(supt, _supte_hash_func, _supte_less_func, NULL);
  return supt;
}

void supt_destroy(supt_t supt) {

  hash_destroy(supt, _supte_free_func);
  free(supt);
}

struct supte *supte_lookup_page(supt_t supt, void *upage) {
  struct supte lookup_e;
  lookup_e.upage = upage;
  struct hash_elem *elem = hash_find(supt, &lookup_e.hash_elem);
  if(elem == NULL) return NULL;

  return hash_entry(elem, struct supte, hash_elem);
}

bool supt_load_page(supt_t supt, uint32_t *pd, void *upage) {
  // is entry available?
  struct supte *supte = supte_lookup_page(supt, upage);
  if(supte == NULL) return false;

  // get me free page
  void *frame_page = frame_alloc(PAL_USER);
  if(frame_page == NULL) return false;

  // load the page with data
  switch(supte->tag) {
  case P_ZERO:
    memset(frame_page, 0, PGSIZE);
    break;
  case P_SWAP:
    swap_in(supte->data.swap.swap_idx, frame_page);
    break;
  case P_FILE:
    PANIC("LOAD FROM FILE!!");
    break;
  }

  bool writable = supte->writable;

  // translation remapping
  if(!pagedir_set_page(pd, upage, frame_page, writable)) {
    frame_free_hard(pd, frame_page);
    return false;
  }

  pagedir_set_dirty(pd, frame_page, false);

  return true;
}

bool supt_install_zero_page(supt_t supt, void *upage) {
  struct supte *supte = malloc(sizeof *supte);
  ASSERT(supte != NULL);

  supte->upage = upage;
  supte->tag = P_ZERO;
  supte->writable = true;

  struct hash_elem *pe;
  pe = hash_insert(supt, &supte->hash_elem);
  if(pe != NULL) PANIC("not expecting duplicate entry");

  return true;
}

static unsigned _supte_hash_func(const struct hash_elem *e, UNUSED void *aux) {
  struct supte *supte = hash_entry(e, struct supte, hash_elem);
  return hash_bytes(&supte->upage, sizeof supte->upage);
}

static bool _supte_less_func(struct hash_elem const *a, struct hash_elem const *b, UNUSED void *aux) {
  struct supte *entry_a, *entry_b;
  entry_a = hash_entry(a, struct supte, hash_elem);
  entry_b = hash_entry(b, struct supte, hash_elem);
  
  return entry_a->upage < entry_b->upage;
}
static void _supte_free_func(struct hash_elem *e, UNUSED void *aux) {
  struct supte *supte = hash_entry(e, struct supte, hash_elem);

  if(supte->tag == P_SWAP) {
    swap_slot_free(supte->data.swap.swap_idx);
  }

  free(supte);
}
