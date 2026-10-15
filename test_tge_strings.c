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
}


// #define U_STR(S) U ## S
// #define u8_STR(S) u8 ## S
//
// int
// main (void)
// {
//   TGE_GC_LOCAL_ROOT (utf32_t, u32);
//   TGE_GC_LOCAL_ROOT (utf8_t, u8);
//   TGE_GC_LOCAL_ROOT (char8_t *, str1);
//   TGE_GC_LOCAL_ROOT (char8_t *, str2);
//
//   u32 = make_utf32 (U_STR ("eĥoŝanĝo ĉiuĵaŭde"));
//   u8 = make_utf8 (u8_STR ("eĥoŝanĝo ĉiuĵaŭde"));
//   str1 = utf32_to_c8str (u32);
//   str2 = utf8_to_c8str (u8);
//   assert (strcmp (str1, "eĥoŝanĝo ĉiuĵaŭde") == 0);s
//   assert (strcmp (str1, str2) == 0);
//   assert (utf32_cmp (u32, u32) == 0);
//   assert (utf32_cmp (u32, utf8_to_utf32 (u8)) == 0);
//   assert (utf8_cmp (u8, utf32_to_utf8 (u32)) == 0);
//   assert (utf32_cmp (make_utf32 (U_STR ("alpaca")), u32) < 0);
//   assert (utf8_cmp (make_utf8 (u8_STR ("alpaca")), u8) < 0);
//   assert (utf32_cmp (u32, make_utf32 (U_STR ("alpaca"))) > 0);
//   assert (utf8_cmp (u8, make_utf8 (u8_STR ("alpaca"))) > 0);
//
//   u32 = make_utf32 (U_STR ("eĥoŝanĝo ĉiuĵaŭde"));
//   u8 = utf32_to_utf8 (u32);
//   str1 = utf32_to_c8str (u32);
//   str2 = utf8_to_c8str (u8);
//   assert (strcmp (str1, str2) == 0);
//
//   u8 = make_utf8 (u8_STR ("eĥoŝanĝo ĉiuĵaŭde"));
//   u32 = utf8_to_utf32 (u8);
//   str1 = utf32_to_c8str (u32);
//   str2 = utf8_to_c8str (u8);
//   assert (strcmp (str1, str2) == 0);
//
//   TGE_GC_UNREGISTER_ROOT (u32);
//   TGE_GC_UNREGISTER_ROOT (u8);
//   TGE_GC_UNREGISTER_ROOT (str1);
//   TGE_GC_UNREGISTER_ROOT (str2);
// }
