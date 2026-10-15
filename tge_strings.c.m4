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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>
#include <threads.h>
#include <locale.h>
#include <tge_xalloc.h>
//#include <spookyhash.h>
#include <tge_strings.h>

// #if HAVE_GETENTROPY
// #include <unistd.h>
// #endif

//----------------------------------------------------------------------

static size_t
char32_literal_length (char32_t CPTR s)
{
  size_t i = 0;
  while (s[i] != 0)
    i += 1;
  return i;
}

static size_t
char8_literal_length (char8_t CPTR s)
{
  return strlen ((char CPTR) s);
}

//----------------------------------------------------------------------

once_flag utf8_locale_is_initialized = ONCE_FLAG_INIT;
static locale_t utf8_locale;

static void
initialize_utf8_locale ()
{
  utf8_locale = newlocale (LC_ALL_MASK, "C.UTF-8", (locale_t) 0);
  if (utf8_locale == (locale_t) 0)
    {
      perror ("creation of C.UTF-8 locale hath failed us");
      abort ();
    }
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
  locale_t saved_locale;

  call_once (PTR_TO utf8_locale_is_initialized, initialize_utf8_locale);
  saved_locale = uselocale (utf8_locale);

  memset (PTR_TO state, 0, sizeof (state));
  size_t i_src = 0;
  size_t i_dst = 0;
  while (i_src != n_src)
    {
      char tmp_buf[MB_LEN_MAX] = { 0 };
      size_t n = c32rtomb (tmp_buf, src[i_src], PTR_TO state);
      if (n == (size_t) -1)
        {
          // Use the REPLACEMENT CHARACTER U+FFFD.
          memset (PTR_TO state, 0, sizeof (state));
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
  PUT (num_written) = i_dst;

  uselocale (saved_locale);
}

static void
convert_utf8_to_utf32_string (char32_t PTR restrict dst,
                              size_t n_dst,
                              char8_t CPTR src,
                              size_t n_src,
                              size_t PTR restrict num_written)
{
  assert (dst != nullptr);
  assert (src != nullptr);
  assert (num_written != nullptr);
  assert (n_src <= n_dst);

  mbstate_t state;
  locale_t saved_locale;

  call_once (PTR_TO utf8_locale_is_initialized, initialize_utf8_locale);
  saved_locale = uselocale (utf8_locale);

  memset (PTR_TO state, 0, sizeof (state));
  size_t i_src = 0;
  size_t i_dst = 0;
  while (i_src != n_src)
    {
      char32_t codept = 0;
      size_t n = mbrtoc32 (PTR_TO codept, PTR_ADD (src, i_src),
                           n_src - i_src, PTR_TO state);
      if (n == 0)
        {
          dst[i_dst] = U'\0';
          i_src += 1;
        }
      else if (n == (size_t) -1 || n == (size_t) -2)
        {
          // Use the REPLACEMENT CHARACTER U+FFFD.
          memset (PTR_TO state, 0, sizeof (state));
          dst[i_dst] = U'\uFFFD';
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
  PUT (num_written) = i_dst;

  uselocale (saved_locale);
}

//----------------------------------------------------------------------

//m4_define([:m4_utfNN_implementation:],
//[:

TGE_VISIBLE extern inline size_t utf$1_len (utf$1_t u$1);
TGE_VISIBLE extern inline char$1_t CPTR utf$1_str (utf$1_t u$1);

TGE_VISIBLE utf$1_t
make_utf$1_n (const char$1_t PTR s, size_t n)
{
  const size_t nbytes_header = sizeof (struct utf$1_data__);
  const size_t nbytes_string = n * sizeof (char$1_t);
  const size_t nbytes = nbytes_header + nbytes_string;

  struct utf$1_data__ PTR buf;
  TGE_MALLOCBUF (buf, nbytes);

  buf->n__ = n;
  memcpy (buf->s__, s, nbytes_string);

  gc_cell_t result = make_gc_object (nullptr, 0, buf, nbytes, free);

  TGE_MALLOCBUF_FREE (buf, nbytes);

  return (utf$1_t) result;
}

TGE_VISIBLE utf$1_t
make_utf$1 (const char$1_t *s)
{
  return make_utf$1_n (s, char$1_literal_length (s));
}

TGE_VISIBLE int
utf$1_cmp (utf$1_t left, utf$1_t right)
{
  int cmp;
  const size_t left_n = utf$1_len (left);
  const size_t right_n = utf$1_len (right);
  if (left_n < right_n)
    cmp = -1;
  else if (right_n < left_n)
    cmp = 1;
  else
    {
      char$1_t CPCTR left_s = utf$1_str (left);
      char$1_t CPCTR right_s = utf$1_str (right);
      cmp = memcmp (left_s, right_s, left_n * sizeof (char$1_t));
    }
  return cmp;
}

// :])

//m4_utfNN_implementation(32);
//m4_utfNN_implementation(8);

//----------------------------------------------------------------------

TGE_VISIBLE utf8_t
utf32_to_utf8 (utf32_t u32)
{
  const size_t u32_n = utf32_len (u32);
  char32_t CPCTR u32_s = utf32_str (u32);
  size_t n_buf = u32_n * MB_LEN_MAX;
  char8_t PTR buf = tge_xmalloc (n_buf * sizeof (char8_t));
  size_t num_written;
  convert_utf32_to_utf8_string (buf, n_buf, u32_s, u32_n,
                                PTR_TO num_written);
  utf8_t u8 = make_utf8_n (buf, num_written);
  free (buf);
  return u8;
}

TGE_VISIBLE utf32_t
utf8_to_utf32 (utf8_t u8)
{
  const size_t u8_n = utf8_len (u8);
  char8_t CPCTR u8_s = utf8_str (u8);
  size_t n_buf = u8_n;
  char32_t PTR buf = tge_xmalloc (n_buf * sizeof (char32_t));
  size_t num_written;
  convert_utf8_to_utf32_string (buf, n_buf, u8_s, u8_n,
                                PTR_TO num_written);
  utf32_t u32 = make_utf32_n (buf, num_written);
  free (buf);
  return u32;
}

//----------------------------------------------------------------------

// TGE_VISIBLE char8_t *
// utf32_to_c8str (utf32_t u32)
// {
//   TGE_GC_LOCAL_ROOT (utf8_t, u8);
//   u8 = utf32_to_utf8 (u32);
//   char8_t *s = utf8_to_c8str (u8);
//   TGE_GC_UNREGISTER_ROOT (u8);
//   return s;
// }
//
// TGE_VISIBLE char8_t *
// utf8_to_c8str (utf8_t u8)
// {
//   char8_t *s = tge_gc_malloc ((u8->n + 1) * sizeof (char8_t));
//   memcpy (s, u8->s, u8->n);
//   s[u8->n] = 0;
//   return s;
// }

//----------------------------------------------------------------------

// once_flag crypto_seed_is_initialized = ONCE_FLAG_INIT;
// static uint64_t crypto_seed;
//
// void
// initialize_crypto_seed (void)
// {
// #if HAVE_GETENTROPY
//   getentropy (PTR_TO crypto_seed, sizeof (crypto_seed));
// #else
//   crypto_seed = 0;
// #endif
// }
//
// struct utf32_t_hash_context
// {
//   utf32_t str;
//   uint64_t *hashes;             /* Memoized hashes. */
//   size_t num_hashes;
// };
//
// static const uint64_t utf32_t_hash_this_first[2] = {
//   // A 128-bit, cryptographically sound random number, generated by
//   // the following command:
//   //
//   //   head -c 16 /dev/urandom | xxd -p
//   //
//   [0] = 0x34c29f0b735af80c,
//   [1] = 0xbf82f80a171cba29
// };
//
// TGE_VISIBLE utf32_t_hash_context_t
// utf32_t_hash_init (utf32_t str)
// {
//   utf32_t_hash_context_t context =
//     tge_gc_malloc (sizeof (struct utf32_t_hash_context));
//   context->str = str;
//   context->hashes = NULL;
//   context->num_hashes = 0;
//   return context;
// }
//
// static inline void
// check_utf32_t_hash_index (unsigned int i)
// {
//   /* Put this here to make the program “terminating in principle” when
//      things like ideal hashmaps do the practically impossible. */
//   if (i == ((unsigned int) INT_MAX) + 1)
//     {
//       fprintf (stderr, "utf32_t_hash index out of bounds: %u\n", i);
//       abort ();
//     }
// }
//
// static void
// copy_old_utf32_t_hashes (uint64_t *new_hashes,
//                          utf32_t_hash_context_t context)
// {
//   if (context->hashes != NULL)
//     memcpy (new_hashes, context->hashes,
//             context->num_hashes * sizeof (uint64_t));
// }
//
// static void
// compute_new_utf32_t_hashes (uint64_t *new_hashes,
//                             size_t new_num_hashes,
//                             utf32_t_hash_context_t context)
// {
//   /* Memoize all the hashes up to new_num_hashes.
//
//      Memoizing all the hashes (rather than doing anything more
//      complicated) presumes algorithms will go progressively from hash
//      0, to hash 1, to hash 2, and so on. Thus, in practice, there will
//      be no hashes computed and simply thrown away. */
//
//   spookyhash_context_t boo;
//
//   call_once (PTR_TO crypto_seed_is_initialized, initialize_crypto_seed);
//
//   for (size_t i = context->num_hashes; i != new_num_hashes; i += 2)
//     {
//       uint64_t seed1 = i / 2;
//       uint64_t seed2 = crypto_seed;
//       spookyhash_init (PTR_TO boo, seed1, seed2);
//       spookyhash_update (PTR_TO boo, utf32_t_hash_this_first,
//                          2 * sizeof (uint64_t));
//       spookyhash_update (PTR_TO boo, context->str->s,
//                          context->str->n * sizeof (uint32_t));
//       spookyhash_final (PTR_TO boo, PTR_TO new_hashes[i], PTR_TO new_hashes[i + 1]);
//     }
// }
//
// TGE_VISIBLE uint64_t
// utf32_t_hash (utf32_t_hash_context_t context, unsigned int i)
// {
//   check_utf32_t_hash_index (i);
//
//   /* j = i rounded to the next higher even. */
//   unsigned int j = ((i & ~1U) + 2);
//
//   if (context->num_hashes < j)
//     {
//       uint64_t *new_hashes = tge_gc_malloc (j * sizeof (uint64_t));
//       copy_old_utf32_t_hashes (new_hashes, context);
//       compute_new_utf32_t_hashes (new_hashes, j, context);
//       context->hashes = new_hashes;
//       context->num_hashes = j;
//     }
//
//   return context->hashes[i];
// }

//----------------------------------------------------------------------
// local variables:
// mode: c
// coding: utf-8
// end:
