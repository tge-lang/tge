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

#include "tge_xalloc.c"
#include "tge_gc.c"
#include <stdio.h>

static size_t
tge_gc_test_get_registered_root_count (void)
{
  size_t count = 0;
  for (tge_gc_root_node_t curr = GC.roots_head;
       curr != nullptr; curr = curr->next)
    count += 1;
  return count;
}

static inline size_t
tge_gc_test_get_total_allocated_bytes (void)
{
  return GC.total_allocated;
}

struct test_node;
typedef struct test_node *test_node_t;
struct test_node
{
  int payload;
  test_node_t next;
};

#define TEST_ASSERT(condition, message)                         \
  if (!(condition))                                             \
    {                                                           \
      fprintf (stderr, "REGRESSION FAILURE: %s\n", message);    \
      failures += 1;                                            \
    }

static int
run_garbage_collector_regression_suite (void)
{
  int failures = 0;
  tge_gc_init ();

  printf
    ("[1] Testing base operational initialization parameters...\n");
  TEST_ASSERT (tge_gc_test_get_registered_root_count () == 0,
               "Roots array must initialize empty.");
  TEST_ASSERT (tge_gc_test_get_total_allocated_bytes () == 0,
               "Allocated balance metric must initialize at zero.");

  printf ("[2] Testing isolated root linkage expansion behaviors...\n");
  {
    TGE_GC_LOCAL_ROOT (test_node_t, first_root);
    TGE_GC_LOCAL_ROOT (test_node_t, second_root);

    TEST_ASSERT (tge_gc_test_get_registered_root_count () == 2,
                 "Dynamic chain length step mismatched.");

    first_root =
      (test_node_t) tge_gc_malloc (sizeof (struct test_node));
    second_root =
      (test_node_t) tge_gc_malloc (sizeof (struct test_node));

    TEST_ASSERT (tge_gc_test_get_total_allocated_bytes () ==
                 (sizeof (struct test_node) * 2),
                 "Allocation sizes incorrectly metrics-tracked.");

    tge_gc_collect ();
    TEST_ASSERT (tge_gc_test_get_total_allocated_bytes () ==
                 (sizeof (struct test_node) * 2),
                 "Active roots mistakenly swept during cycle.");

    TGE_GC_UNREGISTER_ROOT (second_root);
    TGE_GC_UNREGISTER_ROOT (first_root);
  }

  printf ("[3] Validating scope collapse cleanup sweeps...\n");
  tge_gc_collect ();
  TEST_ASSERT (tge_gc_test_get_total_allocated_bytes () == 0,
               "Orphaned nested block heap "
               "entries leaked memory frames.");
  TEST_ASSERT (tge_gc_test_get_registered_root_count () == 0,
               "Spliced variable references failed "
               "to clean up link headers.");

  printf
    ("[4] Evaluating complex multi-tier deep reference graphs...\n");
  {
    TGE_GC_LOCAL_ROOT (test_node_t, head_root);
    head_root = (test_node_t) tge_gc_malloc (sizeof (struct test_node));

    if (head_root != nullptr)
      {
        head_root->payload = 777;
        head_root->next =
          (test_node_t) tge_gc_malloc (sizeof (struct test_node));
        if (head_root->next != nullptr)
          head_root->next->payload = 888;
      }

    tge_gc_collect ();
    TEST_ASSERT (tge_gc_test_get_total_allocated_bytes () ==
                 (sizeof (struct test_node) * 2),
                 "Graph children structures prematurely collected.");

    if (head_root != nullptr)
      head_root->next = nullptr;

    tge_gc_collect ();
    TEST_ASSERT (tge_gc_test_get_total_allocated_bytes () ==
                 sizeof (struct test_node),
                 "Severed internal layout elements leaked heap space.");

    TGE_GC_UNREGISTER_ROOT (head_root);
  }

  printf ("[5] Checking multi-tiered cyclic graph loops...\n");
  {
    TGE_GC_LOCAL_ROOT (test_node_t, node_a);
    TGE_GC_LOCAL_ROOT (test_node_t, node_b);

    node_a = (test_node_t) tge_gc_malloc (sizeof (struct test_node));
    node_b = (test_node_t) tge_gc_malloc (sizeof (struct test_node));

    if ((node_a != nullptr) * (node_b != nullptr))
      {
        node_a->payload = 111;
        node_b->payload = 222;

        /* Create a circular dependency cycle. */
        node_a->next = node_b;
        node_b->next = node_a;
      }

    tge_gc_collect ();
    TEST_ASSERT (tge_gc_test_get_total_allocated_bytes () ==
                 (sizeof (struct test_node) * 2),
                 "Cyclic roots corrupted graph tracking "
                 "mapping matrix.");

    TGE_GC_UNREGISTER_ROOT (node_b);
    tge_gc_collect ();
    TEST_ASSERT (tge_gc_test_get_total_allocated_bytes () ==
                 (sizeof (struct test_node) * 2),
                 "Cycle elements prematurely swept when "
                 "single link remained active.");

    TGE_GC_UNREGISTER_ROOT (node_a);
  }

  tge_gc_collect ();
  TEST_ASSERT (tge_gc_test_get_total_allocated_bytes () == 0,
               "Post-execution remaining bytes must resolve to zero.");

  return failures;
}

int
main (void)
{
  int structural_faults = run_garbage_collector_regression_suite ();

  if (structural_faults == 0)
    printf ("\nSUCCESS\n");
  else
    fprintf (stderr, "\nFAILURE: %d faults\n", structural_faults);

  return structural_faults;
}
