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

//----------------------------------------------------------------------

#ifndef TGE__SPOOKYHASH_H__INCLUDED__
#define TGE__SPOOKYHASH_H__INCLUDED__

#include <stdint.h>
#include <stddef.h>

typedef struct
{
  /* Unhashed data, for partial messages. */
  uint64_t data[24];
  /* Internal state of the hash. */
  uint64_t state[12];
  /* Total length of the input so far. */
  size_t length;
  /* Length of unhashed data stashed in data. */
  uint8_t remainder;
} spookyhash_context_t;

/*
  spookyhash_init:

  Initialize the context of a SpookyHash.

  Any 64-bit value, including zero, will work as a seed. Different
  seeds produce hashes independent of each other.
*/
void spookyhash_init (spookyhash_context_t * context,
                      uint64_t seed1, uint64_t seed2);

/*
  spookyhash_update:

  Add a message fragment to a SpookyHash context.
*/
void spookyhash_update (spookyhash_context_t * context,
                        const void *message, size_t length);

/*
  spookyhash_final:

  Compute the hash for the current context.

  *hash1 = the first 64 bits, in native byte order
  *hash2 = the second 64 bits, in native byte order

  Either hash1 or hash2 may be set to nullptr.

  You can continue updating the context, even after
  you have used spookyhash_final.
*/
void spookyhash_final (spookyhash_context_t * context,
                       uint64_t *hash1, uint64_t *hash2);

/*
  spookyhash_bytes:

  Fills hash_bytes with bits of the (native byte order) 128-bit hash
  value, in little-endian order. (Little-endian is the order in which
  the original message was interpreted.)
*/
void spookyhash_bytes (uint64_t hash1, uint64_t hash2,
                       uint8_t hash_bytes[2 * sizeof (uint64_t)]);

/*
  spookyhash_final_bytes:

  Combines spookyhash_final and spookyhash_bytes.
*/
void spookyhash_final_bytes (spookyhash_context_t * context,
                             uint8_t hash_bytes[2 * sizeof (uint64_t)]);

#endif /* TGE__SPOOKYHASH_H__INCLUDED__ */

//----------------------------------------------------------------------
// local variables:
// mode: c
// coding: utf-8
// end:
