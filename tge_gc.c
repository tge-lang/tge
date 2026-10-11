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

  const size_t managed_count;
  const size_t resource_count;
  void (PCTR finalizer) (void CPPTR handles_array, size_t count);

  void PTR handles[];
};

typedef struct
{
  struct gc_cell PPTR data;
  size_t size;
  size_t capacity;
} object_vector;

typedef enum:unsigned char
{
  gc_idle,
  gc_marking,
  gc_sweeping
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

static thread_local gc_engine global_gc = { };

static void
vector_push (object_vector PTR target_vector,
             struct gc_cell PTR tracking_object)
{
  if (target_vector->size >= target_vector->capacity)
    {
      target_vector->capacity =
        target_vector->capacity == 0 ? 4 : target_vector->capacity * 2;
      target_vector->data =
        tge_xrealloc (target_vector->data,
                      target_vector->capacity *
                      sizeof (struct gc_cell PTR));
    }
  target_vector->data[target_vector->size] = tracking_object;
  target_vector->size += 1;
}

static bool
vector_is_empty (const object_vector PTR target_vector)
{
  bool is_empty = (target_vector->size == 0);
  return is_empty;
}

static struct gc_cell PTR
vector_top (const object_vector PTR target_vector)
{
  struct gc_cell PTR top_element =
    target_vector->data[target_vector->size - 1];
  return top_element;
}

static void
vector_pop (object_vector PTR target_vector)
{
  target_vector->size -= 1;
}

static void
vector_clear (object_vector PTR target_vector)
{
  free (target_vector->data);
  target_vector->data = nullptr;
  target_vector->size = 0;
  target_vector->capacity = 0;
}

TGE_VISIBLE void CPCPTR
gc_get_handles_ptr (gc_cell_t current_cell)
{
  void CPCPTR tracking_array = (void CPCPTR) current_cell->handles;
  return tracking_array;
}

TGE_VISIBLE size_t
gc_get_resource_count (gc_cell_t current_cell)
{
  return current_cell->resource_count;
}

TGE_VISIBLE size_t
gc_get_managed_count (gc_cell_t current_cell)
{
  return current_cell->managed_count;
}

TGE_VISIBLE size_t
gc_get_allocated_bytes_ ()
{
  return global_gc.allocated_bytes;
}

TGE_VISIBLE size_t
gc_get_heap_size_ ()
{
  return global_gc.all_objects.size;
}

TGE_VISIBLE size_t
gc_get_roots_size_ ()
{
  return global_gc.roots.size;
}

TGE_VISIBLE unsigned char
gc_get_phase_ ()
{
  return (unsigned char) global_gc.phase;
}

TGE_VISIBLE void
gc_set_density_threshold (size_t threshold)
{
  global_gc.density_threshold = threshold;
}

TGE_VISIBLE void
gc_reset_engine_ ()
{
  vector_clear (PTR_TO global_gc.all_objects);
  vector_clear (PTR_TO global_gc.gray_stack);
  vector_clear (PTR_TO global_gc.roots);
  global_gc.allocated_bytes = 0;
  global_gc.sweep_index = 0;
  global_gc.phase = gc_idle;
}

TGE_VISIBLE void
auto_run_deferred_func (void PTR function_slot_address)
{
  void (PTR PTR internal_func) (void PTR) =
    (void (PTR PTR) (void PTR)) function_slot_address;
  if (internal_func != nullptr)
    if (PTR internal_func != nullptr)
      (PTR internal_func) (nullptr);
}

TGE_VISIBLE void
register_gc_root (gc_root_handle_t handle, // <------- FIXME: GET RID OF THIS
                  gc_cell_t cell)
{
  struct gc_root_handle PTR mutable_handle =
    (struct gc_root_handle PTR) handle;
  PUT ((struct gc_cell PPTR) (PTR_TO mutable_handle->live_cell)) =
    (struct gc_cell PTR) cell;
  vector_push (PTR_TO global_gc.roots, (struct gc_cell PTR) cell);
}

TGE_VISIBLE void
auto_unregister_gc_root (gc_root_handle_t handle)
{
  size_t match_index = global_gc.roots.size;
  for (size_t i = 0; i < global_gc.roots.size; i += 1)
    if (global_gc.roots.data[i] == handle->live_cell)
      match_index = i;

  if (match_index < global_gc.roots.size)
    {
      for (size_t j = match_index; j < global_gc.roots.size - 1; j += 1)
        global_gc.roots.data[j] = global_gc.roots.data[j + 1];
      global_gc.roots.size -= 1;
    }
}

TGE_VISIBLE gc_cell_t
make_gc_object (gc_cell_t PTR children, size_t child_capacity,
                void PCPTR resources, size_t resource_capacity,
                void (*cleanup_hook) (void CPPTR, size_t))
{
  size_t total_handles_needed = resource_capacity + child_capacity;
  size_t total_allocation_size =
    sizeof (struct gc_cell) +
    (total_handles_needed * sizeof (void PTR));

  if (global_gc.density_threshold > 0)
    while (global_gc.allocated_bytes + total_allocation_size >
           global_gc.density_threshold)
      {
        gc_step (4, resource_capacity);
        if (global_gc.phase == gc_idle)
          if (global_gc.allocated_bytes + total_allocation_size >
              global_gc.density_threshold)
            global_gc.density_threshold =
              (global_gc.allocated_bytes + total_allocation_size) * 2;
      }

  struct gc_cell PTR allocated_object =
    tge_xmalloc (total_allocation_size);
  void PPTR object_handles_buffer =
    (void PPTR) allocated_object->handles;

  memset (allocated_object, 0, total_allocation_size);
  allocated_object->color = color_white;
  allocated_object->forwarding_address = allocated_object;

  PUT ((size_t PTR) (&allocated_object->resource_count)) =
    resource_capacity;
  PUT ((size_t PTR) (&allocated_object->managed_count)) =
    child_capacity;
  PUT ((void (PPTR) (void CPPTR, size_t))
         (&allocated_object->finalizer)) = cleanup_hook;

  if (resource_capacity > 0)
    memcpy (object_handles_buffer, resources,
            resource_capacity * sizeof (void PTR));
  if (child_capacity > 0)
    memcpy (PTR_TO object_handles_buffer[resource_capacity], children,
            child_capacity * sizeof (struct gc_cell PTR));

  vector_push (PTR_TO global_gc.all_objects, allocated_object);
  global_gc.allocated_bytes += total_allocation_size;
  return (gc_cell_t) allocated_object;
}

// TGE_VISIBLE void
// gc_write_barrier (gc_cell_t source_node,
//                   gc_cell_t target_node)
// {
//   if (source_node->color == color_black)
//     if (target_node->color == color_white)
//       {
//         struct gc_cell PTR target_mutable =
//           (struct gc_cell PTR) target_node;
//         target_mutable->color = color_gray;
//         vector_push (PTR_TO global_gc.gray_stack, target_mutable);
//       }
// }

static void
shade_flat_child (struct gc_cell PTR child_node)
{
  if (child_node != nullptr)
    if (child_node->color == color_white)
      {
        child_node->color = color_gray;
        vector_push (PTR_TO global_gc.gray_stack, child_node);
      }
}

static void
process_gray_node (struct gc_cell PTR processing_node,
                   size_t max_resource_capacity)
{
  void PPTR targeted_handles = (void PPTR) processing_node->handles;
  processing_node->color = color_black;
  for (size_t child_index = 0;
       child_index < processing_node->managed_count; child_index += 1)
    {
      size_t actual_slot = max_resource_capacity + child_index;
      struct gc_cell PTR child_ptr =
        (struct gc_cell PTR) targeted_handles[actual_slot];
      shade_flat_child (child_ptr);
    }
}

static void
purge_and_free_dead_objects (size_t max_resource_capacity)
{
  size_t alive_count = 0;
  size_t updated_total_allocated_bytes = 0;

  for (size_t array_index = 0; array_index < global_gc.all_objects.size;
       array_index += 1)
    {
      struct gc_cell PTR current_object =
        global_gc.all_objects.data[array_index];
      size_t object_slots =
        current_object->resource_count + current_object->managed_count;
      size_t object_byte_size =
        sizeof (struct gc_cell) + (object_slots * sizeof (void PTR));

      if (current_object->marked_for_deletion)
        {
          if (current_object->finalizer != nullptr)
            {
              void CPPTR targeted_handles =
                (void CPPTR) current_object->handles;
              current_object->finalizer (targeted_handles,
                                         current_object->resource_count);
            }
          free (current_object);
        }
      else
        {
          global_gc.all_objects.data[alive_count] = current_object;
          alive_count += 1;
          updated_total_allocated_bytes += object_byte_size;
        }
    }
  global_gc.all_objects.size = alive_count;
  global_gc.allocated_bytes = updated_total_allocated_bytes;
}

static void
start_marking_phase ()
{
  for (size_t array_index = 0; array_index < global_gc.all_objects.size;
       array_index += 1)
    {
      global_gc.all_objects.data[array_index]->color = color_white;
      global_gc.all_objects.data[array_index]->marked_for_deletion =
        false;
    }
  for (size_t root_index = 0; root_index < global_gc.roots.size;
       root_index += 1)
    {
      struct gc_cell PTR local_root = global_gc.roots.data[root_index];
      local_root->color = color_gray;
      vector_push (PTR_TO global_gc.gray_stack, local_root);
    }
  global_gc.phase = gc_marking;
}

static void
run_marking_increment (size_t work_units, size_t max_resource_capacity)
{
  bool increment_active = true;
  for (size_t individual_step = 0; individual_step < work_units;
       individual_step += 1)
    if (increment_active)
      if (vector_is_empty (PTR_TO global_gc.gray_stack))
        {
          global_gc.phase = gc_sweeping;
          global_gc.sweep_index = 0;
          increment_active = false;
        }
      else
        {
          struct gc_cell PTR targets_node =
            vector_top (PTR_TO global_gc.gray_stack);
          vector_pop (PTR_TO global_gc.gray_stack);
          process_gray_node (targets_node, max_resource_capacity);
        }
}

static void
run_sweeping_increment (size_t work_units, size_t max_resource_capacity)
{
  bool increment_active = true;
  for (size_t individual_step = 0; individual_step < work_units;
       individual_step += 1)
    if (increment_active)
      if (global_gc.sweep_index >= global_gc.all_objects.size)
        {
          purge_and_free_dead_objects (max_resource_capacity);
          global_gc.phase = gc_idle;
          increment_active = false;
        }
      else
        {
          struct gc_cell PTR sweep_target =
            global_gc.all_objects.data[global_gc.sweep_index];
          if (sweep_target->color == color_white)
            sweep_target->marked_for_deletion = true;
          global_gc.sweep_index += 1;
        }
}

TGE_VISIBLE void
gc_step (size_t work_units, size_t max_resource_capacity)
{
  if (global_gc.phase == gc_idle)
    start_marking_phase ();
  else if (global_gc.phase == gc_marking)
    run_marking_increment (work_units, max_resource_capacity);
  else if (global_gc.phase == gc_sweeping)
    run_sweeping_increment (work_units, max_resource_capacity);
}
