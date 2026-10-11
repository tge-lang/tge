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
#include <string.h>
#include <threads.h>

typedef struct
{
  size_t total_tests;
  size_t passed_tests;
} test_suite_tracker;

static thread_local test_suite_tracker thread_regression_tracker = { };

static once_flag global_init_flag = ONCE_FLAG_INIT;
static mtx_t console_mutex;
static int global_failure_occurred = 0;

static thread_local int global_finalizer_calls = 0;

static void
assert_true (const char PTR test_name, bool condition)
{
  thread_regression_tracker.total_tests += 1;
  mtx_lock (PTR_TO console_mutex);
  if (condition)
    {
      printf ("[PASS] %s\n", test_name);
      thread_regression_tracker.passed_tests += 1;
    }
  else
    {
      printf ("[FAIL] %s\n", test_name);
      global_failure_occurred = 1;
    }
  mtx_unlock (PTR_TO console_mutex);
}

static void
sample_localized_finalizer (VCPPTR handles_array, size_t count)
{
  for (size_t index = 0; index != count; index += 1)
    free ((VPTR) handles_array[index]);
  global_finalizer_calls += 1;
}

static void
diagnostic_one_time_setup ()
{
  printf ("[INIT] Thread synchronization infrastructure"
          " bound natively via call_once.\n");
  mtx_init (PTR_TO console_mutex, mtx_plain);
}

static void
test_nested_lexical_scope_roots ()
{
  assert_true ("Baseline root tracking stack is empty",
               gc_get_roots_size_ () == 0);
  {
    DECLARE_GC_ROOT outer_frame_root;
    gc_cell_t obj_a =
      make_gc_object (nullptr, 0, nullptr, 0, nullptr);
    register_gc_root (PTR_TO outer_frame_root, obj_a);

    assert_true ("Outer let-frame correctly pushes variable"
                 " pointer to roots stack", gc_get_roots_size_ () == 1);
    {
      DECLARE_GC_ROOT inner_frame_root;
      gc_cell_t obj_b =
        make_gc_object (nullptr, 0, nullptr, 0, nullptr);
      register_gc_root (PTR_TO inner_frame_root, obj_b);

      assert_true ("Inner nested let-frame tracks combined"
                   " concurrent roots", gc_get_roots_size_ () == 2);
    }
    assert_true ("Exiting inner block popped nested"
                 " handle from roots array",
                 gc_get_roots_size_ () == 1);
  }
  assert_true ("Exiting final outer block unrolled active"
               " roots back to zero", gc_get_roots_size_ () == 0);
  gc_reset_engine_ ();
}

static void
test_immutable_allocation_and_linking ()
{
  gc_cell_t child_node =
    make_gc_object (nullptr, 0, nullptr, 0, nullptr);
  gc_cell_t PCTR child_view = (gc_cell_t PTR) PTR_TO (child_node);

  gc_cell_t root_node =
    make_gc_object (child_view, 1, nullptr, 0, nullptr);
  VCPCPTR root_handles = gc_get_handles_ptr (root_node);

  assert_true ("Immutable cell tracked object "
               "children limits via query functions",
               gc_get_managed_count (root_node) == 1);
  assert_true ("Dynamic heap size tracks both created elements",
               gc_get_heap_size_ () == 2);
  assert_true ("Memcpy preserved direct memory mapping "
               "inside FAM payload segment",
               (gc_cell_t) (GET (root_handles)) == child_node);

  gc_reset_engine_ ();
}

static void
test_cache_friendly_marking_traversal ()
{
  gc_cell_t child_node =
    make_gc_object (nullptr, 0, nullptr, 0, nullptr);
  gc_cell_t PTR child_view = PTR_TO child_node;
  gc_cell_t tree_root =
    make_gc_object (child_view, 1, nullptr, 0, nullptr);

  {
    DECLARE_GC_ROOT scope_root;
    register_gc_root (PTR_TO scope_root, tree_root);

    gc_step (1, 0);
    assert_true ("Collector phase stepped forward into marking cycle",
                 gc_get_phase_ () == 1);

    gc_step (1, 0);
    assert_true ("Trace advanced and mapped the internal container",
                 gc_get_phase_ () == 1);
  }
  gc_reset_engine_ ();
}

static void
test_localized_unmanaged_resource_finalization ()
{
  VPTR resource_payload = malloc (16);
  if (!resource_payload)
    exit (EXIT_FAILURE);
  const size_t max_resource_capacity = 1;
  global_finalizer_calls = 0;

  VPCPTR resource_view = (VPCPTR) &resource_payload;
  make_gc_object (nullptr, 0, resource_view, max_resource_capacity,
                  sample_localized_finalizer);
  assert_true ("Allocation heap maps raw node",
               gc_get_heap_size_ () == 1);

  gc_step (1, max_resource_capacity);
  while (gc_get_phase_ () != 0)
    gc_step (1, max_resource_capacity);

  assert_true ("Pure incremental mark-sweep "
               "flushed unanchored memory block",
               gc_get_heap_size_ () == 0);
  assert_true ("Object finalizer callback triggered "
               "from base address index 0",
               global_finalizer_calls == 1);
  gc_reset_engine_ ();
}

static void
test_adaptive_allocation_density_trigger ()
{
  gc_set_density_threshold (200);

  gc_cell_t active_anchor =
    make_gc_object (nullptr, 0, nullptr, 0, nullptr);
  {
    DECLARE_GC_ROOT tracking_anchor;
    register_gc_root (PTR_TO tracking_anchor, active_anchor);

    make_gc_object (nullptr, 0, nullptr, 0, nullptr);
    make_gc_object (nullptr, 0, nullptr, 0, nullptr);
    make_gc_object (nullptr, 0, nullptr, 0, nullptr);

    assert_true ("Adaptive allocation engine bounded memory growth",
                 gc_get_phase_ () != 0 ||
                 gc_get_allocated_bytes_ () < 200);
  }
  gc_set_density_threshold (0);
  gc_reset_engine_ ();
}

static int
concurrent_thread_worker (VPTR arg)
{
  size_t thread_id = (size_t) arg;

  mtx_lock (PTR_TO console_mutex);
  printf ("\n[THREAD %zu] Spawning thread-isolated"
          " execution environment context...\n", thread_id);
  mtx_unlock (&console_mutex);

  thread_regression_tracker.total_tests = 0;
  thread_regression_tracker.passed_tests = 0;

  test_nested_lexical_scope_roots ();
  test_immutable_allocation_and_linking ();
  test_cache_friendly_marking_traversal ();
  test_localized_unmanaged_resource_finalization ();
  test_adaptive_allocation_density_trigger ();

  mtx_lock (PTR_TO console_mutex);
  printf ("[THREAD %zu] Completed: %zu / %zu Checks Passed.\n",
          thread_id, thread_regression_tracker.passed_tests,
          thread_regression_tracker.total_tests);
  mtx_unlock (PTR_TO console_mutex);

  return thrd_success;
}

int
main1 ()
{
  thrd_t first_worker = { };
  thrd_t second_worker = { };

  call_once (PTR_TO global_init_flag, diagnostic_one_time_setup);

  printf ("==================================================\n");
  printf (" RUNNING OPAQUE ABSTRACT DATA TYPE GC SUITE\n");
  printf ("==================================================\n");

  thrd_create (PTR_TO first_worker, concurrent_thread_worker,
               (VPTR) 1);
  thrd_create (PTR_TO second_worker, concurrent_thread_worker,
               (VPTR) 2);
  thrd_join (first_worker, nullptr);
  thrd_join (second_worker, nullptr);

  printf ("==================================================\n");
  if (global_failure_occurred != 0)
    printf (" CONCURRENT HARNESS RUN COMPLETED "
            "WITH REGRESSION FAILURES.\n");
  else
    printf (" CONCURRENT HARNESS RUN EXECUTION "
            "COMPLETED SUCCESSFULLY.\n");
  printf ("==================================================\n");

  mtx_destroy (PTR_TO console_mutex);
  return global_failure_occurred;
}

static void
finalize_string (VCPPTR s, size_t n)
{
  for (size_t i = 0; i != n; i += 1)
    {
      free ((VPTR) s[i]);
      s[i] = nullptr;
    }
  assert_true ("string finalized", true);
}

int
main2 ()
{
  printf ("Testing GC_ROOT_HERE\n");
  {
    VPTR string[1] = { strdup ("test string") };
    gc_cell_t x = make_gc_object (nullptr, 0, string, 1,
                                  finalize_string);
    GC_ROOT_HERE (x);
    for (size_t i = 0; i != 100; i += 1)
      gc_step (1, 1);
    //
    // The following assertion is likely to segfault if the root is
    // not working.
    //
    assert_true ("strcmp ((char CPCTR) (gc_get_handles_ptr (x) [0]), "
                 "\"test string\") == 0)",
                 strcmp ((char CPCTR) (gc_get_handles_ptr (x)[0]),
                         "test string") == 0);
  }
  for (size_t i = 0; i != 100; i += 1)
    gc_step (1, 1);
  return 0;
}

int
main3 ()
{
  printf ("Testing GC_ROOT\n");
  {
    VPTR string[1] = { strdup ("test string") };
    GC_ROOT (gc_cell_t, x, make_gc_object (nullptr, 0, string, 1,
                                           finalize_string));
    for (size_t i = 0; i != 100; i += 1)
      gc_step (1, 1);
    //
    // The following assertion is likely to segfault if the root is
    // not working.
    //
    assert_true ("strcmp ((char CPCTR) (gc_get_handles_ptr (x) [0]), "
                 "\"test string\") == 0)",
                 strcmp ((char CPCTR) (gc_get_handles_ptr (x)[0]),
                         "test string") == 0);
  }
  for (size_t i = 0; i != 100; i += 1)
    gc_step (1, 1);
  return 0;
}

int
main ()
{
  int status = main1 ();
  if (status == 0)
    status = main2 ();
  if (status == 0)
    status = main3 ();
  return status;
}
