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

#ifndef TGE__TGE_GC_H__INCLUDED__
#define TGE__TGE_GC_H__INCLUDED__

//----------------------------------------------------------------------
//
// An incremental garbage collector for persistent (immutable) data
// structures.
//
//----------------------------------------------------------------------

#include <c_syntax.h>
#include <stddef.h>

struct gc_cell;
typedef struct gc_cell CPCTR gc_cell_t;

struct gc_root_handle;
typedef struct gc_root_handle CPCTR gc_root_handle_t;
struct gc_root_handle
{
  gc_cell_t live_cell;
};

void auto_unregister_gc_root (gc_root_handle_t handle);

#define DECLARE_GC_ROOT                                                 \
        [[gnu::cleanup(auto_unregister_gc_root)]] struct gc_root_handle

//
// Call GC_ROOT_HERE(...) as shorthand for declaring a root in a stack
// block. Typically you might declare and initialize a variable
// “varname” and then immediately call “GC_ROOT_HERE(varname);”
//
#define GC_ROOT_HERE(Value) GC_root_here__ (Value, __COUNTER__)
#define GC_root_here_join__(U, V) U ## V
#define GC_root_here_handle__(N)                \
        GC_root_here_join__ (gc_root_here_handle__, N)
#define GC_root_here__(Value, N)                                \
        DECLARE_GC_ROOT GC_root_here_handle__ (N);                     \
        register_gc_root (PTR_TO GC_root_here_handle__ (N), (Value))

//
// Declare and initialize a variable, and root it in a stack block, in
// one step.
//
#define GC_ROOT(Type, Variable, Value) \
        Type Variable = (Value); \
        GC_ROOT_HERE (Variable)

gc_cell_t make_gc_object (gc_cell_t PTR children,
                          size_t child_capacity,
                          void PCPTR resources,
                          size_t resource_capacity,
                          void (*cleanup_hook) (void CPPTR,
                                                size_t));
extern char CPCTR unknown_gc_object_type;

void register_gc_root (gc_root_handle_t handle,
                       gc_cell_t cell);
// void gc_write_barrier (gc_cell_t source_node,
//                        gc_cell_t target_node);
void gc_step (size_t work_units, size_t max_resource_capacity);
void CPCPTR gc_get_handles_ptr (const gc_cell_t current_cell);

size_t gc_get_resource_count (const gc_cell_t current_cell);
size_t gc_get_managed_count (const gc_cell_t current_cell);

size_t gc_get_allocated_bytes_ ();
size_t gc_get_heap_size_ ();
size_t gc_get_roots_size_ ();
unsigned char gc_get_phase_ ();
void gc_set_density_threshold (size_t threshold);
void gc_reset_engine_ ();

#endif /* TGE__TGE_GC_H__INCLUDED__ */
