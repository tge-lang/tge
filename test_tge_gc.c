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
#include <assert.h>

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
sample_localized_finalizer (VPTR data)
{
  free (GET ((VPPTR) data));
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
               gc_roots_size () == 0);
  {
    GC_ROOT (auto, obj_a,
             make_gc_object (nullptr, 0, nullptr, 0, nullptr));

    assert_true ("Outer let-frame correctly pushes variable"
                 " pointer to roots stack", gc_roots_size () == 1);
    {
      GC_ROOT (auto, obj_b,
               make_gc_object (nullptr, 0, nullptr, 0, nullptr));

      assert_true ("Inner nested let-frame tracks combined"
                   " concurrent roots", gc_roots_size () == 2);
    }
    assert_true ("Exiting inner block popped nested"
                 " handle from roots array",
                 gc_roots_size () == 1);
  }
  assert_true ("Exiting final outer block unrolled active"
               " roots back to zero", gc_roots_size () == 0);
  gc_reset_engine ();
}

static void
test_immutable_allocation_and_linking ()
{
  gc_cell_t child_node =
    make_gc_object (nullptr, 0, nullptr, 0, nullptr);
  gc_cell_t PCTR child_view = (gc_cell_t PTR) PTR_TO (child_node);

  gc_cell_t root_node =
    make_gc_object (child_view, 1, nullptr, 0, nullptr);
  VCPTR root_data = gc_data (root_node);

  assert_true ("Immutable cell tracked object "
               "children limits via query functions",
               gc_child_count (root_node) == 1);
  assert_true ("Dynamic heap size tracks both created elements",
               gc_heap_size () == 2);
  assert_true ("Memcpy preserved direct memory mapping "
               "inside attachment",
               (GET ((gc_cell_t PTR) root_data)) == child_node);

  gc_reset_engine ();
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
    GC_ROOT_HERE (auto, tree_root);

    gc_step (1);
    assert_true ("Collector phase stepped forward into marking cycle",
                 gc_phase () == 1);

    gc_step (1);
    assert_true ("Trace advanced and mapped the internal container",
                 gc_phase () == 1);
  }
  gc_reset_engine ();
}

static void
test_localized_unmanaged_resource_finalization ()
{
  constexpr size_t data_size = 16;
  VPTR data = malloc (data_size);
  if (data == nullptr)
    exit (EXIT_FAILURE);
  global_finalizer_calls = 0;

  VPPTR view = PTR_TO data;
  make_gc_object (nullptr, 0, view, sizeof (VPPTR),
                  sample_localized_finalizer);
  assert_true ("Allocation heap maps raw node",
               gc_heap_size () == 1);

  gc_step (1);
  while (gc_phase () != 0)
    gc_step (1);

  assert_true ("Pure incremental mark-sweep "
               "flushed unanchored memory block",
               gc_heap_size () == 0);
  assert_true ("Object finalizer callback triggered "
               "from base address index 0",
               global_finalizer_calls == 1);
  gc_reset_engine ();
}

static void
test_adaptive_allocation_density_trigger ()
{
  gc_set_density_threshold (200);

  gc_cell_t active_anchor =
    make_gc_object (nullptr, 0, nullptr, 0, nullptr);
  {
    GC_ROOT_HERE (auto, active_anchor);

    make_gc_object (nullptr, 0, nullptr, 0, nullptr);
    make_gc_object (nullptr, 0, nullptr, 0, nullptr);
    make_gc_object (nullptr, 0, nullptr, 0, nullptr);

    assert_true ("Adaptive allocation engine bounded memory growth",
                 gc_phase () != 0 ||
                 gc_allocated_bytes () < 200);
  }
  gc_set_density_threshold (0);
  gc_reset_engine ();
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
threaded_tests ()
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

int
main ()
{
  return threaded_tests ();
}
