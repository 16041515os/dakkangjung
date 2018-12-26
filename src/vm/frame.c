#include "frame.h"

#include <list.h>
#include <hash.h>
#include "lib/debug.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"

#include <stdio.h>

#include "swap.h"

/* frame table entry */
struct fte {
  // hash key
  void *kpage;

  // hash value
  /* initially NULL, meaning KPAGE has no mapping */
  void *upage;
  struct thread *thread;

  // initially true on creation
  bool pinned;
  bool writable;

  // for hashmap
  struct hash_elem hash_elem;
  // for linear fte tranversal
  struct list_elem list_elem;
};

static struct hash frame_table;
static struct list frame_list;
static struct list_elem *frame_list_iter;

struct lock frame_lock;

static unsigned _frame_hash_func(struct hash_elem const *e, void *aux);
static bool _frame_less_func(struct hash_elem const *a, struct hash_elem const *b, void *aux);

static struct fte *next_frame(void);
static struct fte *select_victim_frame(void);

static void frame_free_hard_internal(uint32_t *pd, void *kpage);
static void frame_entry_free_internal(uint32_t *pd, void *kpage);

void frame_init(void) {
  hash_init(&frame_table, _frame_hash_func, _frame_less_func, NULL);
  list_init(&frame_list);
  frame_list_iter = NULL;

  lock_init(&frame_lock);
}

void *frame_alloc(struct thread *thread, enum palloc_flags pflags) {
  ASSERT((pflags & PAL_USER) != 0);

  lock_acquire(&frame_lock);

  void *new_frame = palloc_get_page(pflags);
  if(new_frame == NULL) {
    PANIC("NO FRAME !!!");
    struct fte *victim = select_victim_frame();
    struct thread *victim_thr = victim->thread;

    pagedir_clear_page(victim_thr->pagedir, victim->upage);

    uint32_t swap_idx = swap_out(victim->kpage);
    supt_install_swap_page(victim_thr->supt, victim->upage, swap_idx, victim->writable);

    frame_free_hard_internal(victim_thr->pagedir, victim->kpage);

    new_frame = palloc_get_page(pflags);
    ASSERT(new_frame != NULL);
  }

  struct fte *entry = malloc(sizeof *entry);
  ASSERT(entry != NULL);

  entry->kpage = new_frame;
  entry->upage = NULL;
  entry->thread = thread;
  entry->pinned = true;
  entry->writable = false;

  hash_insert(&frame_table, &entry->hash_elem);
  list_push_back(&frame_list, &entry->list_elem);

  lock_release(&frame_lock);

  return new_frame;
}

void frame_set_page(uint32_t *pd, void *upage, void *kpage, bool writable) {
  lock_acquire(&frame_lock);

  ASSERT(pagedir_get_page(pd, upage) == kpage);

  struct fte lookup_e;
  lookup_e.kpage = kpage;
  struct hash_elem *found = hash_find(&frame_table, &lookup_e.hash_elem);
  if(found == NULL) {
    PANIC("- frame_set_page: no frame entry for KPAGE");
  }

  struct fte *entry = hash_entry(found, struct fte, hash_elem);
  ASSERT(pd == entry->thread->pagedir);

  entry->upage = upage;
  entry->writable = writable;

  lock_release(&frame_lock);
}

void frame_free_hard(uint32_t *pd, void *kpage) {
  lock_acquire(&frame_lock);
  frame_free_hard_internal(pd, kpage);
  lock_release(&frame_lock);
}

void frame_entry_free(uint32_t *pd, void *kpage) {
  lock_acquire(&frame_lock);
  frame_entry_free_internal(pd, kpage);
  lock_release(&frame_lock);
}

static void frame_free_hard_internal(uint32_t *pd, void *kpage) {
  frame_entry_free_internal(pd, kpage);
  palloc_free_page(kpage);
}

static void frame_entry_free_internal(uint32_t *pd, void *kpage) {
  ASSERT(is_kernel_vaddr(kpage));
  ASSERT(pg_ofs(kpage) == 0);

  struct fte lookup_e, *found_e;
  lookup_e.kpage = kpage;
  struct hash_elem *found = hash_find(&frame_table, &lookup_e.hash_elem);
  if(found == NULL) {
    PANIC("- frame_entry_free: KPAGE not found on FRAME_TABLE");
  }

  found_e = hash_entry(found, struct fte, hash_elem);
  ASSERT(found_e->thread->pagedir == pd);

  hash_delete(&frame_table, &found_e->hash_elem);
  list_remove(&found_e->list_elem);

  free(found_e);
}

void frame_set_pinned(void *kpage, bool value) {
  ASSERT(is_kernel_vaddr(kpage));
  ASSERT(pg_ofs(kpage) == 0);

  lock_acquire(&frame_lock);

  struct fte lookup_e, *found_e;
  lookup_e.kpage = kpage;
  struct hash_elem *found = hash_find(&frame_table, &lookup_e.hash_elem);
  if(found == NULL) {
    PANIC("- frame_set_pinned: KPAGE not found on FRAME_TABLE");
  }

  found_e = hash_entry(found, struct fte, hash_elem);
  found_e->pinned = value;

  lock_release(&frame_lock);
}

static struct fte *next_frame() {
  if (list_empty(&frame_list))
    PANIC("Frame table is empty");

  if(frame_list_iter == NULL || frame_list_iter == list_end(&frame_list))
    frame_list_iter = list_begin(&frame_list);
  else
    frame_list_iter = list_next(frame_list_iter);
  
  if(frame_list_iter == list_end(&frame_list)) frame_list_iter = list_begin(&frame_list);
  
  return list_entry(frame_list_iter, struct fte, list_elem);
}

static struct fte *select_victim_frame() {
  size_t len = hash_size(&frame_table);
  ASSERT(len > 0);

  size_t i;
  for(i=0; i < len<<1; ++i) {
    struct fte *entry = next_frame();
    if(entry->pinned) continue;

    if(pagedir_is_accessed(entry->thread->pagedir, entry->upage)) {
      pagedir_set_accessed(entry->thread->pagedir, entry->upage, false);
      continue;
    }

    return entry;

  }
  PANIC("no available frame for victim");
  return NULL;
}

static unsigned _frame_hash_func(struct hash_elem const *e, UNUSED void *aux) {
  struct fte *fte = hash_entry(e, struct fte, hash_elem);
  return hash_bytes(&fte->kpage, sizeof fte->kpage);
}

static bool _frame_less_func(struct hash_elem const *a, struct hash_elem const *b, UNUSED void *aux) {
  struct fte *fte_a, *fte_b;
  fte_a = hash_entry(a, struct fte, hash_elem);
  fte_b = hash_entry(b, struct fte, hash_elem);
  return fte_a->kpage < fte_b->kpage;
}
