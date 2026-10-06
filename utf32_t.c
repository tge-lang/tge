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

#include <assert.h>
#include <uchar.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>
#include <xalloc.h>
#include <utf32_t.h>

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
#else
#error "This code assumes C23 or greater."
#endif

#define UTF32_NULL_CHARACTER (U'\0')
#define UTF32_REPLACEMENT_CHARACTER (U'\uFFFD')

//----------------------------------------------------------------------

struct utf32
{
  size_t n;
  char32_t s[];
};

struct utf8
{
  size_t n;
  char8_t s[];
};

//----------------------------------------------------------------------

static size_t
char32_literal_length (const char32_t *s)
{
  size_t i = 0;
  while (s[i] != 0)
    i += 1;
  return i;
}

static void
convert_utf32_to_utf8_string (char8_t *restrict dst,
                              size_t n_dst,
                              const char32_t *restrict src,
                              size_t n_src,
                              size_t *restrict num_written)
{
  assert (dst != nullptr);
  assert (src != nullptr);
  assert (num_written != nullptr);
  assert (n_src * MB_LEN_MAX <= n_dst);

  mbstate_t state;

  memset (&state, 0, sizeof (state));
  size_t i_src = 0;
  size_t i_dst = 0;
  while (i_src != n_src)
    {
      char tmp_buf[MB_LEN_MAX] = { 0 };
      size_t n = c32rtomb (tmp_buf, src[i_src], &state);
      if (n == (size_t) -1)
        {
          // Use the REPLACEMENT CHARACTER U+FFFD.
          memset (&state, 0, sizeof (state));
          tmp_buf[0] = 0xEF;
          tmp_buf[1] = 0xBF;
          tmp_buf[2] = 0xBD;
          n = 3;
        }
      for (size_t i = 0; i != n; i += 1)
        dst[i_dst + i] = tmp_buf[i];
      i_dst += n;
      i_src += 1;
    }
  *num_written = i_dst;
}

static void
convert_utf8_to_utf32_string (char32_t *restrict dst,
                              size_t n_dst,
                              const char8_t *src,
                              size_t n_src,
                              size_t *restrict num_written)
{
  assert (dst != nullptr);
  assert (src != nullptr);
  assert (num_written != nullptr);
  assert (n_src <= n_dst);

  mbstate_t state;

  memset (&state, 0, sizeof (state));
  size_t i_src = 0;
  size_t i_dst = 0;
  while (i_src != n_src)
    {
      char32_t codept = 0;
      size_t n = mbrtoc32 (&codept, &src[i_src], n_src - i_src, &state);
      if (n == 0)
        {
          dst[i_dst] = UTF32_NULL_CHARACTER;
          i_src += 1;
        }
      else if (n == (size_t) -1 || n == (size_t) -2)
        {
          // Use the REPLACEMENT CHARACTER U+FFFD.
          memset (&state, 0, sizeof (state));
          dst[i_dst] = UTF32_REPLACEMENT_CHARACTER;
          i_src += 1;
        }
      else if (n == (size_t) -3)
        dst[i_dst] = codept;
      else
        {
          dst[i_dst] = codept;
          i_src += n;
        }
      i_dst += 1;
    }
  *num_written = i_dst;
}

//----------------------------------------------------------------------

TGE_VISIBLE utf32_t
make_utf32_n (const char32_t *s, size_t n)
{
  struct utf32 *u32 =
    xmalloc (sizeof (struct utf32) + (n * sizeof (char32_t)));
  u32->n = n;
  memcpy (u32->s, s, n * sizeof (char32_t));
  return u32;
}

TGE_VISIBLE utf32_t
make_utf32 (const char32_t *s)
{
  return make_utf32_n (s, char32_literal_length (s));
}

TGE_VISIBLE utf8_t
make_utf8_n (const char8_t *s, size_t n)
{
  struct utf8 *u8 =
    xmalloc (sizeof (struct utf8) + (n * sizeof (char8_t)));
  u8->n = n;
  memcpy (u8->s, s, n * sizeof (char8_t));
  return u8;
}

TGE_VISIBLE utf8_t
make_utf8 (const char8_t *s)
{
  return make_utf8_n (s, strlen (s));
}

//----------------------------------------------------------------------

TGE_VISIBLE utf8_t
utf32_to_utf8 (utf32_t u32)
{
  size_t n_buf = u32->n * MB_LEN_MAX;
  char8_t *buf = xmalloc (n_buf * sizeof (char8_t));
  size_t num_written;
  convert_utf32_to_utf8_string (buf, n_buf, u32->s, u32->n,
                                &num_written);
  utf8_t u8 = make_utf8_n (buf, num_written);
  free (buf);
  return u8;
}

TGE_VISIBLE utf32_t
utf8_to_utf32 (utf8_t u8)
{
  size_t n_buf = u8->n;
  char32_t *buf = xmalloc (n_buf * sizeof (char32_t));
  size_t num_written;
  convert_utf8_to_utf32_string (buf, n_buf, u8->s, u8->n, &num_written);
  utf32_t u32 = make_utf32_n (buf, num_written);
  free (buf);
  return u32;
}

//----------------------------------------------------------------------
// local variables:
// mode: c
// coding: utf-8
// end:
