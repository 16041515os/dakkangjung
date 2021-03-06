#include "page.h"

#include <stdio.h>
#include <debug.h>
#include <string.h>
#include <threads/thread.h>
#include "threads/malloc.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "filesys/file.h"
#include "frame.h"
#include "swap.h"


static unsigned _supte_hash_func(const struct hash_elem *e, void *aux);
static bool _supte_less_func(struct hash_elem const *a, struct hash_elem const *b, void *aux);
static void _supte_free_func(struct hash_elem *e, void *aux);

static bool load_page_from_file(struct supte *supte, uint8_t *kpage);

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

void *supt_load_page(struct thread *thread, void *upage) {
  uint32_t *pd = thread->pagedir;

  ASSERT(pagedir_get_page(pd, upage) == NULL);

  // is entry available?
  struct supte *supte = supte_lookup_page(thread->supt, upage);
  if(supte == NULL) return NULL;

  // get me free page
  void *frame_page = frame_alloc(thread, PAL_USER);
  if(frame_page == NULL) {
    PANIC("FRAME UNAVAILABLE");
    return NULL;
  }

  // load the page with data
  switch(supte->tag) {
  case P_ZERO:
    // printf("from zero\n");
    memset(frame_page, 0x00, PGSIZE);
    break;
  case P_SWAP:
    // printf("from swap\n");
    swap_in(supte->data.swap.swap_idx, frame_page);
    break;
  case P_FILE:
    // printf("from file\n");
    if(!load_page_from_file(supte, frame_page)) {
      frame_free_hard(thread->pagedir, frame_page);
      return false;
    }
    break;
  }

  bool writable = supte->writable;

  // translation remapping
  if(!pagedir_set_page(pd, upage, frame_page, writable)) {
    PANIC("mapping should be DONE");
    return NULL;
  }

  hash_delete(thread->supt, &supte->hash_elem);
  _supte_free_func(&supte->hash_elem, NULL);
  
  pagedir_set_dirty(pd, frame_page, false);

  frame_set_pinned(frame_page, false);

  return frame_page;
}

bool supt_install_swap_page(supt_t supt, void *upage, uint32_t swap_idx, bool writable) {
  struct supte *supte = malloc(sizeof *supte);
  ASSERT(supte != NULL);

  supte->upage = upage;
  supte->tag = P_SWAP;
  supte->data.swap.swap_idx = swap_idx;
  supte->writable = writable;

  struct hash_elem *pe;
  pe = hash_insert(supt, &supte->hash_elem);
  if(pe != NULL) {
    PANIC("not expecting duplicate entry");
  }

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
  if(pe != NULL) {
    PANIC("not expecting duplicate entry");
  }

  return true;
}

bool supt_install_file(supt_t supt, void *upage,
  struct file *file, off_t offset, uint32_t read_bytes, uint32_t zero_bytes, bool writable
) {
  struct supte *supte = malloc(sizeof *supte);
  ASSERT(supte != NULL);

  supte->upage = upage;
  supte->tag = P_FILE;
  supte->data.file.file = file;
  supte->data.file.ofs = offset;
  supte->data.file.read_bytes = read_bytes;
  supte->data.file.zero_bytes = zero_bytes;
  supte->writable = writable;

  struct hash_elem *pe;
  pe = hash_insert(supt, &supte->hash_elem);
  if(pe != NULL) PANIC("not expecting duplicate entry");

  return true;
}

static bool load_page_from_file(struct supte *supte, uint8_t *kpage) {
  struct file *file = supte->data.file.file;
  off_t ofs = supte->data.file.ofs;
  uint32_t read_bytes = supte->data.file.read_bytes;
  uint32_t zero_bytes = supte->data.file.zero_bytes;

  file_seek(file, ofs);

  uint32_t bytes = file_read(file, kpage, read_bytes);
  if(bytes < read_bytes) return false;

  memset(kpage + read_bytes, 0x00, zero_bytes);
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
