#include <list.h>
#include <hash.h>
#include "lib/debug.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "threads/vaddr.h"

#include "frame.h"

/* frame table entry */
struct fte {
  // hash key
  void *kpage;

  // hash value
  /* initially NULL, meaning KPAGE has no mapping */
  void *upage;
  uint32_t *pagedir;

  // initially true on creation
  bool pinned;

  // for hashmap
  struct hash_elem hash_elem;
  // for linear fte tranversal
  struct list_elem list_elem;
};

static struct hash frame_table;
static struct list frame_list;

struct lock frame_lock;

static unsigned _frame_hash_func(struct hash_elem const *e, void *aux);
static bool _frame_less_func(struct hash_elem const *a, struct hash_elem const *b, void *aux);

void frame_init(void) {
  hash_init(&frame_table, _frame_hash_func, _frame_less_func, NULL);
  list_init(&frame_list);

  lock_init(&frame_lock);
}

void *frame_alloc(enum palloc_flags pflags) {
  ASSERT((pflags & PAL_USER) != 0);

  lock_acquire(&frame_lock);

  void *new_frame = palloc_get_page(pflags);
  if(new_frame == NULL) {
    PANIC("- frame_alloc: swapping required");
  }

  struct fte *entry = malloc(sizeof *entry);
  ASSERT(entry != NULL);

  entry->kpage = new_frame;
  entry->upage = NULL;
  entry->pagedir = NULL;
  entry->pinned = true;

  hash_insert(&frame_table, &entry->hash_elem);
  list_push_back(&frame_list, &entry->list_elem);

  lock_release(&frame_lock);

  return new_frame;
}

void frame_set_page(uint32_t *pd, void *upage, void *kpage) {
  lock_acquire(&frame_lock);

  struct fte lookup_e;
  lookup_e.kpage = kpage;
  struct hash_elem *found = hash_find(&frame_table, &lookup_e.hash_elem);
  if(found == NULL) {
    PANIC("- frame_set_page: no frame entry for KPAGE");
  }

  struct fte *entry = hash_entry(found, struct fte, hash_elem);

  entry->upage = upage;
  entry->pagedir = pd;

  lock_release(&frame_lock);
}

void frame_entry_free(uint32_t *pd, void *kpage) {
  ASSERT(is_kernel_vaddr(kpage));
  ASSERT(pg_ofs(kpage));

  lock_acquire(&frame_lock);

  struct fte lookup_e, *found_e;
  lookup_e.kpage = kpage;
  struct hash_elem *found = hash_find(&frame_table, &lookup_e.hash_elem);
  if(found == NULL) {
    PANIC("- frame_entry_free: KPAGE not found on FRAME_TABLE");
  }

  found_e = hash_entry(found, struct fte, hash_elem);
  ASSERT(found_e->pagedir == pd);

  hash_delete(&frame_table, &found_e->hash_elem);
  list_remove(&found_e->list_elem);

  free(found_e);

  lock_release(&frame_lock);
}

void frame_set_pinned(void *kpage, bool value) {
  ASSERT(is_kernel_vaddr(kpage));
  ASSERT(pg_ofs(kpage));

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

static unsigned _frame_hash_func(struct hash_elem const *e, UNUSED void *aux) {
  struct fte *fte = hash_entry(e, struct fte, hash_elem);
  return hash_bytes(fte->kpage, sizeof fte->kpage);
}

static bool _frame_less_func(struct hash_elem const *a, struct hash_elem const *b, UNUSED void *aux) {
  struct fte *fte_a, *fte_b;
  fte_a = hash_entry(a, struct fte, hash_elem);
  fte_b = hash_entry(b, struct fte, hash_elem);
  return fte_a->kpage < fte_b->kpage;
}
