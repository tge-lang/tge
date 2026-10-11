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

#include <tge_xalloc.h>
#include <stdlib.h>
#include <stdio.h>

[[noreturn]] static void
tge_xalloc_memory_exhausted ()
{
  fprintf (stderr, "virtual memory exhausted\n");
  abort ();
}

TGE_VISIBLE void PTR
tge_xmalloc (size_t n)
{
  if (n == 0)
    n = 1;
  void *p = malloc (n);
  if (p == nullptr)
    tge_xalloc_memory_exhausted ();
  return p;
}

TGE_VISIBLE void PTR
tge_xrealloc (void PTR p0, size_t n)
{
  void PTR p;
  if (p0 == nullptr || n == 0)
    p = tge_xmalloc (n);
  else
    {
      p = realloc (p0, n);
      if (p == nullptr)
        tge_xalloc_memory_exhausted ();
    }
  return p;
}
