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
#include <tge_xalloc.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//----------------------------------------------------------------------

typedef enum:unsigned char
{
  color_white,
  color_gray,
  color_black
} gc_color;

struct gc_cell
{
  gc_color color:2;
  unsigned int marked_for_deletion:1;

  struct gc_cell PTR forwarding_address;

  const size_t child_count;
  const size_t data_size;
  gc_finalizer_t PCTR finalizer;

  // A byte array that first has the data, then padding, then the
  // children.
  alignas (max_align_t) unsigned char attachments[];
};

typedef struct
{
  struct gc_cell PPTR data;
  size_t length;
  size_t capacity;
} object_vector;

typedef enum:unsigned char
{
  gc_idle = 0,
  gc_marking = 1,
  gc_sweeping = 2
} gc_phase_state;

typedef struct
{
  object_vector all_objects;
  object_vector gray_stack;
  object_vector roots;
  size_t sweep_index;
  gc_phase_state phase;

  size_t allocated_bytes;
  size_t density_threshold;
} gc_engine;

//----------------------------------------------------------------------

static thread_local gc_engine global_gc = { };

//----------------------------------------------------------------------

static void
vector_push (object_vector PTR vec,
             struct gc_cell PTR tracking_object)
{
  static constexpr size_t min_capacity = 4;

  if (vec->capacity == vec->length)
    {
      vec->capacity = ((vec->capacity < min_capacity) ?
                       min_capacity : (vec->capacity * 2));
      vec->data =
        tge_xrealloc (vec->data, vec->capacity * sizeof vec->data[0]);
    }
  vec->data[vec->length] = tracking_object;
  vec->length += 1;
}

static inline bool
vector_is_empty (const object_vector PTR vec)
{
  return (vec->length == 0);
}

static inline struct gc_cell PTR
vector_top (const object_vector PTR vec)
{
  return vec->data[vec->length - 1];
}

static inline void
vector_pop (object_vector PTR vec)
{
  vec->length -= 1;
}

static void
vector_clear (object_vector PTR vec)
{
  free (vec->data);
  vec->data = nullptr;
  vec->length = 0;
  vec->capacity = 0;
}

//----------------------------------------------------------------------

static inline size_t
gc_cell_t_equivalent (size_t data_size)
{
  // Return the number of gc_cell_t required to hold a well aligned
  // byte array of size data_size.
  return ((data_size / sizeof (gc_cell_t))
          + ((data_size % sizeof (gc_cell_t)) != 0));
}

static inline size_t
gc_cell_and_attachment_size (gc_cell_t cell)
{
  return ((sizeof (struct gc_cell))
          + ((sizeof (gc_cell_t))
             * (gc_cell_t_equivalent (cell->data_size)
                + cell->child_count)));
}

static inline struct gc_cell PPTR
gc_children (gc_cell_t cell)
{
  return ((struct gc_cell PPTR)
          PTR_ADD (cell->attachments,
                   gc_cell_t_equivalent (cell->data_size)));
}

//----------------------------------------------------------------------

TGE_VISIBLE VCPTR
gc_data (gc_cell_t cell)
{
  return (VCPTR) (cell->attachments);
}

TGE_VISIBLE size_t
gc_data_size (gc_cell_t cell)
{
  return cell->data_size;
}

TGE_VISIBLE size_t
gc_child_count (gc_cell_t cell)
{
  return cell->child_count;
}

TGE_VISIBLE void
gc_set_density_threshold (size_t threshold)
{
  global_gc.density_threshold = threshold;
}

TGE_VISIBLE void
register_gc_root (gc_cell_t cell)
{
  vector_push (PTR_TO global_gc.roots, (struct gc_cell PTR) cell);
}

static size_t
find_root (gc_cell_t cell)
{
  size_t k = ((size_t) -1);
  size_t i = 0;
  while ((k == ((size_t) -1)) * (i != global_gc.roots.length))
    if (global_gc.roots.data[i] == cell)
      k = i;
    else
      i += 1;
  return k;
}

TGE_VISIBLE void
deregister_gc_root (gc_cell_t cell)
{
  size_t i = find_root (cell);
  if (i != ((size_t) -1))
    {
      memmove (PTR_ADD (global_gc.roots.data, i),
               PTR_ADD (global_gc.roots.data, i + 1),
               ((global_gc.roots.length - i - 1)
                * sizeof global_gc.roots.data[0]));
      global_gc.roots.length -= 1;
    }
}

static void
do_nothing_finalizer (void PTR unused [[maybe_unused]])
{
  // Do nothing.
}

static gc_finalizer_t PTR
choose_finalizer (gc_finalizer_t PTR finalizer)
{
  return (finalizer == nullptr) ? do_nothing_finalizer : finalizer;
}

TGE_VISIBLE gc_cell_t
make_gc_object (gc_cell_t PTR children, size_t child_capacity,
                void PTR data, size_t data_size,
                gc_finalizer_t PTR finalizer)
{
  constexpr size_t work_units = 4;

  size_t total_size =
    sizeof (struct gc_cell) +
    ((gc_cell_t_equivalent (data_size) + child_capacity)
     * sizeof (gc_cell_t));

  if (global_gc.density_threshold != 0)
    while (global_gc.density_threshold
           < (global_gc.allocated_bytes + total_size))
      {
        gc_step (work_units);
        if ((global_gc.phase == gc_idle) *
            (global_gc.density_threshold
             < (global_gc.allocated_bytes + total_size)))
          global_gc.density_threshold =
            (global_gc.allocated_bytes + total_size) * 2;
      }

  struct gc_cell PTR result = tge_xmalloc (total_size);
  unsigned char PTR data_buffer = result->attachments;

  memset (result, 0, total_size);
  result->color = color_white;
  result->forwarding_address = result;

  PUT ((size_t PTR) (PTR_TO result->data_size)) = data_size;
  PUT ((size_t PTR) (PTR_TO result->child_count)) = child_capacity;
  PUT ((gc_finalizer_t PPTR) (PTR_TO result->finalizer)) =
    choose_finalizer (finalizer);

  if (data_size != 0)
    memcpy (data_buffer, data, data_size);
  if (child_capacity != 0)
    memcpy (PTR_TO data_buffer[data_size], children,
            child_capacity * sizeof (gc_cell_t));

  vector_push (PTR_TO global_gc.all_objects, result);
  global_gc.allocated_bytes += total_size;

  return (gc_cell_t) result;
}

static void
start_marking_phase ()
{
  //
  // Mark everything white, except the roots, which we mark gray and
  // put in a stack.
  //
  for (size_t i = 0; i != global_gc.all_objects.length; i += 1)
    {
      global_gc.all_objects.data[i]->color = color_white;
      global_gc.all_objects.data[i]->marked_for_deletion = false;
    }
  for (size_t i = 0; i != global_gc.roots.length; i += 1)
    {
      struct gc_cell PTR local_root = global_gc.roots.data[i];
      local_root->color = color_gray;
      vector_push (PTR_TO global_gc.gray_stack, local_root);
    }
  global_gc.phase = gc_marking;
}

static void
process_gray_node (struct gc_cell PTR node)
{
  node->color = color_black;

  struct gc_cell PPTR children = gc_children (node);
  for (size_t i = 0; i != node->child_count; i += 1)
    if (children[i] != nullptr && children[i]->color == color_white)
      {
        ((struct gc_cell PTR) children[i])->color = color_gray;
        vector_push (PTR_TO global_gc.gray_stack, children[i]);
      }
}

static void
run_marking_increment (size_t work_units)
{
  //
  // Do at most work_units of work.
  //
  // Go through the gray nodes and progressively paint them first gray
  // and then black.
  //
  size_t i = 0;
  while ((i != work_units)
         * (!vector_is_empty (PTR_TO global_gc.gray_stack)))
    {
      struct gc_cell PTR targets_node =
        vector_top (PTR_TO global_gc.gray_stack);
      vector_pop (PTR_TO global_gc.gray_stack);
      process_gray_node (targets_node);
      i += 1;
    }
  if (i != work_units)
    {
      global_gc.phase = gc_sweeping;
      global_gc.sweep_index = 0;
    }
}

static void
purge_and_free_obsolete_objects ()
{
  size_t alive_count = 0;
  size_t total_bytes = 0;

  for (size_t i = 0; i != global_gc.all_objects.length; i += 1)
    {
      struct gc_cell PTR obj = global_gc.all_objects.data[i];
      if (obj->marked_for_deletion)
        {
          obj->finalizer ((VPTR) obj->attachments);
          free (obj);
        }
      else
        {
          global_gc.all_objects.data[alive_count] = obj;
          alive_count += 1;
          total_bytes += gc_cell_and_attachment_size (obj);
        }
    }

  global_gc.all_objects.length = alive_count;
  global_gc.allocated_bytes = total_bytes;
}

static void
run_sweeping_increment (size_t work_units)
{
  //
  // Do at most work_units of work.
  //
  // Sweep up white objects.
  //
  size_t i = 0;
  while ((i != work_units)
         * (global_gc.sweep_index < global_gc.all_objects.length))
    {
      struct gc_cell PTR sweep_target =
        global_gc.all_objects.data[global_gc.sweep_index];
      if (sweep_target->color == color_white)
        sweep_target->marked_for_deletion = true;
      global_gc.sweep_index += 1;
      i += 1;
    }
  if (i != work_units)
    {
      purge_and_free_obsolete_objects ();
      global_gc.phase = gc_idle;
    }
}

TGE_VISIBLE void
gc_step (size_t work_units)
{
  //
  // A state machine doing at most work_units of work.
  //
  if (global_gc.phase == gc_idle)
    start_marking_phase ();
  else if (global_gc.phase == gc_marking)
    run_marking_increment (work_units);
  else if (global_gc.phase == gc_sweeping)
    run_sweeping_increment (work_units);
}

//----------------------------------------------------------------------
//
// Functions used by the regression test.
//

static size_t
gc_allocated_bytes ()
{
  return global_gc.allocated_bytes;
}

static size_t
gc_heap_size ()
{
  return global_gc.all_objects.length;
}

static size_t
gc_roots_size ()
{
  return global_gc.roots.length;
}

static unsigned char
gc_phase ()
{
  return (unsigned char) global_gc.phase;
}

static void
gc_reset_engine ()
{
  vector_clear (PTR_TO global_gc.all_objects);
  vector_clear (PTR_TO global_gc.gray_stack);
  vector_clear (PTR_TO global_gc.roots);
  global_gc.allocated_bytes = 0;
  global_gc.sweep_index = 0;
  global_gc.phase = gc_idle;
}

//----------------------------------------------------------------------
