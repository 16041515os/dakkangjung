#include "swap.h"

#include <bitmap.h>
#include "threads/vaddr.h"
#include "devices/block.h"

static struct block *SWAP_BLOCK;
static struct bitmap *swap_bitmap;

#define SECTORS_PER_PAGE (PGSIZE / BLOCK_SECTOR_SIZE)

void swap_init() {
  SWAP_BLOCK = block_get_role(BLOCK_SWAP);
  ASSERT(SWAP_BLOCK != NULL);

  size_t swap_pages = block_size(SWAP_BLOCK) / SECTORS_PER_PAGE;
  swap_bitmap = bitmap_create(swap_pages);
  ASSERT(swap_bitmap != NULL);

  bitmap_set_all(swap_bitmap, true);
}

uint32_t swap_out(void *kpage) {
  ASSERT(is_kernel_vaddr(kpage));

  size_t swap_idx = bitmap_scan_and_flip(swap_bitmap, 0, 1, true);
  size_t i;
  size_t sec_idx;
  uint8_t *dest = kpage;
  for(i = 0, sec_idx = swap_idx * SECTORS_PER_PAGE; i < SECTORS_PER_PAGE; ++i, ++sec_idx) {
    block_write(SWAP_BLOCK, sec_idx, dest);
    dest += BLOCK_SECTOR_SIZE;
  }

  return swap_idx;
}

void swap_in(uint32_t swap_idx, void *kpage) {
  ASSERT(is_kernel_vaddr(kpage));
  ASSERT(bitmap_test(swap_bitmap, swap_idx) == false);

  size_t i;
  size_t sec_idx;
  uint8_t *dest = kpage;
  for(i = 0, sec_idx = swap_idx * SECTORS_PER_PAGE; i < SECTORS_PER_PAGE; ++i, ++sec_idx) {
    block_read(SWAP_BLOCK, sec_idx, dest);
    dest += BLOCK_SECTOR_SIZE;
  }

  bitmap_set(swap_bitmap, swap_idx, true);
}

void swap_slot_free(uint32_t swap_idx) {
  ASSERT(bitmap_test(swap_bitmap, swap_idx) == false);

  bitmap_set(swap_bitmap, swap_idx, true);
}