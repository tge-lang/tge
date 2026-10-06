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

#include <uchar.h>
#include <string.h>
#include <stdlib.h>
#include <xalloc.h>
#include <string_t.h>

struct string
{
  size_t n;
  char32_t s[];
};

static size_t
char32_literal_length (const char32_t *s)
{
  size_t i = 0;
  while (s[i] != 0)
    i += 1;
  return i;
}

TGE_VISIBLE string_t
make_string_t (const char32_t *s)
{
  size_t n = char32_literal_length (s);
  struct string *str =
    xmalloc (sizeof (struct string) + (n * sizeof (char32_t)));
  str->n = n;
  memcpy (str->s, s, n * sizeof (char32_t));
  return str;
}
