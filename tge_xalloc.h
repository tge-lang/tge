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

#ifndef TGE__TGE_XALLOC_H__INCLUDED__
#define TGE__TGE_XALLOC_H__INCLUDED__

#include <c_syntax.h>
#include <stdlib.h>

VPTR tge_xmalloc (size_t);
VPTR tge_xrealloc (VPTR, size_t);

//
// TGE_MALLOCBUF(size)
// TGE_MALLOCBUF_FREE(buf, size)
//
// Allocate a temporary buffer that if small will be on the stack.
// The alloca(3) function is not used.
//
#define TGE_MALLOCBUF(buf, size) Tge_mallocbuf__ (buf, size, \
                                                  __COUNTER__)
#define TGE_MALLOCBUF_FREE(buf, size) Tge_mallocbuf_free__ (buf, size)
//
#ifndef TGE_MALLOCBUF_TRANSITION_SIZE
#define TGE_MALLOCBUF_TRANSITION_SIZE 256
#endif
//
#define Tge_mallocbuf__buf__(N) tge_mallocbuf__buf__ ## N
#define Tge_mallocbuf__(buf, size, N)                           \
        unsigned char                                                 \
        Tge_mallocbuf__buf__ (N)[(TGE_MALLOCBUF_TRANSITION_SIZE)];     \
        do                                                            \
          {                                                           \
            buf = (((size) <= (TGE_MALLOCBUF_TRANSITION_SIZE))        \
             ? ((VPTR) Tge_mallocbuf__buf__ (N))                 \
             : ((VPTR) tge_xmalloc ((size))));                  \
          }                                                           \
        while (0)
#define Tge_mallocbuf_free__(buf, size)                 \
        do                                                    \
          {                                                   \
            if ((TGE_MALLOCBUF_TRANSITION_SIZE) < (size))     \
            free ((buf));                                   \
          }                                                   \
        while (0)

#endif // TGE__TGE_XALLOC_H__INCLUDED__
