// Copyright © 2026 Barry Schwartz
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <tge_gc.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <threads.h>
#include <stdckdint.h>

#define TGE_UWB(N) N##uwb

//----------------------------------------------------------------------

static void
tge_xalloc_memory_exhausted (void)
{
  fprintf (stderr, "virtual memory exhausted\n");
  abort ();
}

static inline void *
tge_xmalloc (size_t n)
{
  void *p = malloc (n);
  if (p == nullptr)
    tge_xalloc_memory_exhausted ();
  return p;
}

//----------------------------------------------------------------------

struct allocation_header;
typedef struct allocation_header *allocation_header_t;
struct allocation_header
{
  allocation_header_t next;
  size_t size;
  bool marked;
};

typedef struct
{
  allocation_header_t head;
  size_t total_allocated;
  size_t threshold;
  /* Head of the dynamic linked list of registered variables */
  tge_gc_root_node_t roots_head;
} GarbageCollector;

static thread_local GarbageCollector GC = {
  .head = nullptr,
  .total_allocated = 0,
  .threshold = 1024 * 1024,     /* 1 MB per-thread threshold */
  .roots_head = nullptr
};

TGE_VISIBLE void
tge_gc_init (void)
{
  GC.head = nullptr;
  GC.total_allocated = 0;
  GC.roots_head = nullptr;
}

TGE_VISIBLE void
tge_gc_link_root (tge_gc_root_node_t node, void **root_ptr)
{
  if (node != nullptr)
    {
      node->root_ptr = root_ptr;
      node->next = GC.roots_head;
      GC.roots_head = node;
    }
}

TGE_VISIBLE void
tge_gc_unlink_root (tge_gc_root_node_t node)
{
  tge_gc_root_node_t curr = GC.roots_head;
  tge_gc_root_node_t prev = nullptr;
  bool found = false;
  while (curr != nullptr)
    {
      if (curr == node)
        found = true;
      if (!found)
        prev = curr;
      curr = curr->next;
    }
  if (found)
    {
      if (prev == nullptr)
        GC.roots_head = node->next;
      else
        prev->next = node->next;
    }
}

static void
tge_gc_mark_block (uintptr_t ptr_val)
{
  allocation_header_t curr = GC.head;
  while (curr != nullptr)
    {
      uintptr_t block_start =
        (uintptr_t) ((char *) curr + sizeof (struct allocation_header));
      uintptr_t block_end = block_start + curr->size;

      /* If the value points inside an unmarked block, mark it and
         scan its contents. */
      if (!curr->marked
          && ((block_start <= ptr_val) * (ptr_val < block_end)))
        {
          curr->marked = true;

          /* Align the payload scan boundaries to word sizes
             safely. */
          size_t aligned_size =
            curr->size & ~(sizeof (uintptr_t) - TGE_UWB (1));
          uintptr_t *payload_ptr = (uintptr_t *) block_start;
          uintptr_t *payload_limit =
            (uintptr_t *) ((char *) block_start + aligned_size);

          while (payload_ptr < payload_limit)
            {
              tge_gc_mark_block (*payload_ptr);
              payload_ptr += 1;
            }
        }
      curr = curr->next;
    }
}

static void
tge_gc_sweep (void)
{
  allocation_header_t curr = GC.head;
  allocation_header_t prev = nullptr;

  while (curr != nullptr)
    {
      allocation_header_t next_block = curr->next;

      if (!curr->marked)
        {
          if (prev == nullptr)
            GC.head = next_block;
          else
            prev->next = next_block;
          GC.total_allocated -= curr->size;
          free (curr);
        }
      else
        {
          curr->marked = false;
          prev = curr;
        }
      curr = next_block;
    }
}

TGE_VISIBLE void
tge_gc_collect (void)
{
  tge_gc_root_node_t curr_root = GC.roots_head;

  /* Dynamically trace the active links linked along the thread
     stacks. */
  while (curr_root != nullptr)
    {
      if (curr_root->root_ptr != nullptr)
        {
          void *root_val = *(curr_root->root_ptr);
          if (root_val != nullptr)
            tge_gc_mark_block ((uintptr_t) root_val);
        }
      curr_root = curr_root->next;
    }

  tge_gc_sweep ();
}

TGE_VISIBLE void *
tge_gc_alloc (size_t size)
{
  size_t next_allocation_total;
  size_t total_size;

  void *user_ptr = nullptr;

  bool overflow =
    ckd_add (&next_allocation_total, GC.total_allocated, size);
  if (overflow || next_allocation_total > GC.threshold)
    tge_gc_collect ();

  overflow = ckd_add (&next_allocation_total, GC.total_allocated, size);
  if (!overflow)
    {
      overflow =
        ckd_add (&total_size, sizeof (struct allocation_header), size);
      if (!overflow)
        {
          allocation_header_t block =
            (allocation_header_t) tge_xmalloc (total_size);
          if (block != nullptr)
            {
              block->size = size;
              block->marked = false;
              block->next = GC.head;
              GC.head = block;
              GC.total_allocated = next_allocation_total;
              user_ptr =
                (void *) ((char *) block +
                          sizeof (struct allocation_header));
            }
        }
    }

  return user_ptr;
}

//----------------------------------------------------------------------

#if TGE_GC_DEMO_PROGRAM

typedef struct Node
{
  int data;
  struct Node *link;
} Node;

static int
worker_thread_execution (void *arg)
{
  TGE_GC_LOCAL_ROOT (Node *, current_node);
  TGE_GC_LOCAL_ROOT (Node *, temporary_node);

  tge_gc_init ();
  uintptr_t id = (uintptr_t) arg;

  current_node = (Node *) tge_gc_alloc (sizeof (Node));
  if (current_node != nullptr)
    {
      current_node->data = (int) (id * TGE_UWB (100));

      temporary_node = (Node *) tge_gc_alloc (sizeof (Node));
      if (temporary_node != nullptr)
        {
          temporary_node->data = current_node->data + 50;
          current_node->link = temporary_node;
        }
    }

  printf ("Thread %zu: Allocated values: node=%d\n", id,
          current_node->data);

  tge_gc_collect ();

  current_node = nullptr;
  temporary_node = nullptr;
  tge_gc_collect ();

  TGE_GC_UNREGISTER_ROOT (temporary_node);
  TGE_GC_UNREGISTER_ROOT (current_node);
  return thrd_success;
}

int
main (void)
{
  thrd_t workers[4];
  uintptr_t index = 0;
  int status;

  printf ("Main Launcher: Executing thread pool "
          "with dynamic stack-linked roots...\n");

  while (index < 4)
    {
      status = thrd_create (&workers[index], worker_thread_execution,
                            (void *) index);
      if (status != thrd_success)
        exit (1);
      index += 1;
    }

  index = 0;
  while (index < 4)
    {
      thrd_join (workers[index], &status);
      index += 1;
    }

  printf ("Main Launcher: Library operations completed "
          "successfully with zero buffer limits.\n");
  exit (0);
}

#endif /* TGE_GC_DEMO_PROGRAM */

//----------------------------------------------------------------------
