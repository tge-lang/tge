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

#ifndef TGE__TGE_STRINGS_H__INCLUDED__
#define TGE__TGE_STRINGS_H__INCLUDED__

#include <uchar.h>
#include <stdint.h>
#include <tge_gc.h>

//----------------------------------------------------------------------

//m4_define([:m4_utfNN_declaration:],
//[:

struct utf$1;
typedef struct utf$1 CPTR utf$1_t;

struct utf$1_data__
{
  size_t n__;
  char$1_t s__[];
};
typedef struct utf$1_data__ CPTR utf$1_data_t__;

inline size_t
utf$1_len (utf$1_t u$1)
{
  return ((utf$1_data_t__) gc_data ((gc_cell_t) u$1))->n__;
}

inline char$1_t CPTR
utf$1_str (utf$1_t u$1)
{
  return ((utf$1_data_t__) gc_data ((gc_cell_t) u$1))->s__;
}

utf$1_t make_utf$1_n (char$1_t CPTR, size_t);
utf$1_t make_utf$1 (char$1_t CPTR);

// :])

//m4_utfNN_declaration(32);
//m4_utfNN_declaration(8);

//----------------------------------------------------------------------

utf8_t utf32_to_utf8 (utf32_t);
utf32_t utf8_to_utf32 (utf8_t);

//----------------------------------------------------------------------

// char8_t *utf32_to_c8str (utf32_t);
// char8_t *utf8_to_c8str (utf8_t);

//----------------------------------------------------------------------
//
// The following comparatives are useful for ordered data structures
// and for equality tests, but are useless for putting words in
// alphabetic order, etc. They count the shorter of two strings as
// less than the other, and use memcmp(3) to compare strings of equal
// length. They do not perform normalizations.
//

int utf32_cmp (utf32_t, utf32_t);
int utf8_cmp (utf8_t, utf8_t);

//----------------------------------------------------------------------
//
// String hashing.
//

// struct utf32_t_hash_context;
// typedef struct utf32_t_hash_context *utf32_t_hash_context_t;
//
// utf32_t_hash_context_t utf32_t_hash_init (utf32_t str);
//
// uint64_t utf32_t_hash (utf32_t_hash_context_t context, unsigned int i);

//----------------------------------------------------------------------

#endif // TGE__TGE_STRINGS_H__INCLUDED__

// local variables:
// mode: c
// coding: utf-8
// end:
