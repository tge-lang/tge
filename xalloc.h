/*
  Copyright © 2026 Barry Schwartz
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#ifndef TGE__XALLOC_H__INCLUDED__
#define TGE__XALLOC_H__INCLUDED__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void
tge_xalloc_memory_exhausted (void)
{
  fprintf (stderr, "virtual memory exhausted");
  abort ();
}

static inline void *
xmalloc (size_t n)
{
  void *p = malloc (n);
  if (p == nullptr)
    tge_xalloc_memory_exhausted ();
  return p;
}

#define XMALLOC(T) (xmalloc (sizeof (T)))

#endif /* TGE__XALLOC_H__INCLUDED__ */

/*
  local variables:
  mode: c
  coding: utf-8
  end:
*/
