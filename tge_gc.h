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

//
// Call GC_ROOT_HERE(Type, Value) as shorthand for rooting a value in
// a particular context. The Type can be “auto”.
//
#define GC_ROOT_HERE(Type, Value)               \
        Gc_root_here__ (Type, Value, __COUNTER__)
#define Gc_root_here__(Type, Value, N)                  \
        GC_ROOT (Type, Gc_root_here_variable__ (N), (Value))
// A name derived with “head -c 4 /dev/urandom | xxd -p” to make it
// hard to collide with.
#define Gc_root_here_variable__(N) gc_root__0f0cdc7e__ ## N

//
// Declare and initialize a variable, and root its value, in one step.
// The Type can be “auto”.
//
#define GC_ROOT(Type, Variable, Value)                  \
        Type Variable = (Value);                              \
        register_gc_root ((gc_cell_t) Variable);              \
        defer { deregister_gc_root ((gc_cell_t) Variable); }

//----------------------------------------------------------------------
//
// Types.
//

struct gc_cell;
typedef struct gc_cell CPCTR gc_cell_t;

typedef void gc_finalizer_t (VPTR);

//----------------------------------------------------------------------
//
// Functions.
//

void register_gc_root (gc_cell_t cell);
void deregister_gc_root (gc_cell_t cell);

gc_cell_t make_gc_object (gc_cell_t PTR children,
                          size_t child_capacity,
                          void PTR data,
                          size_t data_size,
                          gc_finalizer_t PTR finalizer);

size_t gc_child_count (gc_cell_t);

VCPTR gc_data (gc_cell_t);
size_t gc_data_size (gc_cell_t);

void gc_step (size_t work_units);
void gc_set_density_threshold (size_t threshold);

//----------------------------------------------------------------------

#endif // TGE__TGE_GC_H__INCLUDED__
