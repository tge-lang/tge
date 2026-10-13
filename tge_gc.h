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
// An incremental mark-sweep garbage collector for persistent
// (immutable) data structures.
//
//----------------------------------------------------------------------

#include <c_syntax.h>
#include <stddef.h>

//----------------------------------------------------------------------
//
// Macros.
//

#define DECLARE_GC_ROOT                                                 \
        [[gnu::cleanup(auto_unregister_gc_root)]] struct gc_root

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

//----------------------------------------------------------------------
//
// Types.
//

struct gc_cell;
typedef struct gc_cell CPCTR gc_cell_t;

struct gc_root;
typedef struct gc_root CPCTR gc_root_t;
struct gc_root
{
  gc_cell_t val;
};

typedef void gc_finalizer_t (VPTR);

//----------------------------------------------------------------------
//
// Functions.
//

void register_gc_root (gc_root_t handle, gc_cell_t cell);
void auto_unregister_gc_root (gc_root_t handle);

gc_cell_t make_gc_object (gc_cell_t PTR children,
                          size_t child_capacity,
                          void PTR data,
                          size_t data_size,
                          gc_finalizer_t PTR finalizer);

//
// DECLARE_GC_ROOT rooted_var = make_rooted_gc_object (...);
//    ...
// Do stuff with rooted_var.val;
//    ...
//
struct gc_root
make_rooted_gc_object (gc_cell_t PTR children,
                       size_t child_capacity,
                       void PTR data,
                       size_t data_size,
                       gc_finalizer_t PTR finalizer);

void gc_step (size_t work_units);

size_t gc_child_count (gc_cell_t);

VCPTR gc_data (gc_cell_t);
size_t gc_data_size (gc_cell_t);

void gc_set_density_threshold (size_t threshold);

//----------------------------------------------------------------------

#endif // TGE__TGE_GC_H__INCLUDED__
