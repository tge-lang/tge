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

#include <c_syntax.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int
main ()
{
  FILE PTR ftmp = tmpfile ();
  char s[7] = { 's', 't', 'r', 'i', 'n', 'g', '\0' };

  int i = 0;

  {
    defer
    {
      fclose (ftmp);
      i = 1;
    };

    rewind (ftmp);
    fwrite (s, sizeof (char), 7, ftmp);

    char t[7];
    rewind (ftmp);
    fread (t, sizeof (char), 7, ftmp);
    assert (strcmp (t, "string") == 0);

    assert (i == 0);
  }

  assert (ftell (ftmp) == -1);
  assert (i == 1);
}
