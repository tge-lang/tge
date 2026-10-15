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

#include <tge_strings.h>
#include <string.h>
#include <assert.h>

int
main ()
{
  GC_ROOT (auto, u32a, make_utf32 (U"eĥoŝanĝo ĉiuĵaŭde"));
  GC_ROOT (auto, u8a, make_utf8 (u8"eĥoŝanĝo ĉiuĵaŭde"));
  GC_ROOT (auto, u32b, make_utf32 (U"alpaca"));
  GC_ROOT (auto, u8b, make_utf8 (u8"alpaca"));
  GC_ROOT (auto, u32a1, utf8_to_utf32 (u8a));
  GC_ROOT (auto, u8a1, utf32_to_utf8 (u32a));
  GC_ROOT (auto, u32b1, utf8_to_utf32 (u8b));
  GC_ROOT (auto, u8b1, utf32_to_utf8 (u32b));

  assert (utf32_cmp (u32a, u32a) == 0);
  assert (utf8_cmp (u8a, u8a) == 0);
  assert (utf32_cmp (u32a, u32b) > 0);
  assert (utf32_cmp (u32b, u32a) < 0);
  assert (utf8_cmp (u8a, u8b) > 0);
  assert (utf8_cmp (u8b, u8a) < 0);

  assert (utf32_cmp (u32a, u32a1) == 0);
  assert (utf32_cmp (u32a1, u32a) == 0);
  assert (utf8_cmp (u8a, u8a1) == 0);
  assert (utf8_cmp (u8a1, u8a) == 0);
  assert (utf32_cmp (u32a1, u32b1) > 0);
  assert (utf32_cmp (u32b1, u32a1) < 0);
  assert (utf8_cmp (u8a1, u8b1) > 0);
  assert (utf8_cmp (u8b1, u8a1) < 0);

  assert (strcmp (utf8_str (u8a), "eĥoŝanĝo ĉiuĵaŭde") == 0);
  assert (strcmp (utf8_str (u8a1), "eĥoŝanĝo ĉiuĵaŭde") == 0);
  assert (strcmp (utf8_str (u8b), "alpaca") == 0);
  assert (strcmp (utf8_str (u8b1), "alpaca") == 0);
}
