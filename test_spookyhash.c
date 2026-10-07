/*

  Copyright © 2026 Barry Schwartz
  
  SPDX-License-Identifier: MIT

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "spookyhash-reference.h"
#include "spookyhash.c"

#define LENGTH_MAX 9999

#define SEED1_A 1
#define SEED2_A 2

#define SEED1_B 3
#define SEED2_B 4

static void
fill_message (char *message, size_t length)
{
  for (size_t i = 0; i != length; i += 1)
    message[i] = (char) (unsigned char) ((i + 128) % 256);
}

static void
test_with_seeds (uint64_t seed1, uint64_t seed2, size_t i_start)
{
  char message[LENGTH_MAX + 1];

  size_t i = i_start;
  for (size_t length = 0; length != LENGTH_MAX + 1; length += 1)
    {
      fill_message (message, length);

      spookyhash_context_t context;
      uint8_t nominal[2 * sizeof (uint64_t)];
      uint8_t bytes[2 * sizeof (uint64_t)];

      memcpy (nominal, &reference[i], 2 * sizeof (uint64_t));

      spookyhash_init (&context, seed1, seed2);
      spookyhash_update (&context, message, length);
      spookyhash_final_bytes (&context, bytes);

      for (size_t j = 0; j != 2 * sizeof (uint64_t); j += 1)
        if (bytes[j] != nominal[j])
          {
            printf ("failed at length %zu\n", length);
            abort ();
          }

      i += 1;
    }
}

int
main (int argc, char **argv)
{
  test_with_seeds (SEED1_A, SEED2_A, 0);
  test_with_seeds (SEED1_B, SEED2_B, LENGTH_MAX + 1);
}
