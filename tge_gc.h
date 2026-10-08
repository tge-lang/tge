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

#include <stddef.h>

/* Tracking node instantiated on the caller’s stack frame. */
struct tge_gc_root_node;
typedef struct tge_gc_root_node *tge_gc_root_node_t;
struct tge_gc_root_node
{
  void **root_ptr;
  tge_gc_root_node_t next;
};

//----------------------------------------------------------------------
//
// Public API.
//

void tge_gc_init (void);        /* Initialize the GC. */
void tge_gc_collect (void);     /* Force a collection. */

void tge_gc_link_root (tge_gc_root_node_t node, void **root_ptr);
void tge_gc_unlink_root (tge_gc_root_node_t node);

/**INDENT-OFF**/

/* The program will abort if the underlying malloc(3) runs out of
   memory. */
void *tge_gc_alloc (size_t size) TGE_NODISCARD;

/**INDENT-ONk**/

//----------------------------------------------------------------------
//
// Convenience macros for rooting collectible allocations.
//

#define TGE_GC_LOCAL_ROOT(type, name)       \
  type name = nullptr;                  \
  struct tge_gc_root_node __tge_gc_node_##name; \
  tge_gc_link_root (&__tge_gc_node_##name, (void **) &name)

#define TGE_GC_UNREGISTER_ROOT(name)        \
  tge_gc_unlink_root (&__tge_gc_node_##name)

//----------------------------------------------------------------------

#endif /* TGE__TGE_GC_H__INCLUDED__ */
