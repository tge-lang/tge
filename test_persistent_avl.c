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

#include <persistent_avl.h>
#include <tge_gc.h>
#include <assert.h>
#include <stdlib.h>
#include <locale.h>

#define U_STR(S) U##S
#define u8_STR(S) u8##S

#define BUFSIZE 2000

static int buf[BUFSIZE];

int
int_cmp (const int *a, const int *b, TGE_MAYBE_UNUSED void *ignored)
{
  return (*a < *b) ? -1 : ((*b < *a) ? 1 : 0);
}

void
fisher_yates (void)
{
  for (int i = 0; i != (BUFSIZE); i += 1)
    buf[i] = i;
  for (int i = (BUFSIZE) - 1; i != 0; i -= 1)
    {
      /* Dividing the big random integer is good enough. */
      int j = (int) (random () % ((long) i) + 1);

      int _tmp = buf[i];
      buf[i] = buf[j];
      buf[j] = _tmp;
    }
}

TGE_AVL_NODE_DECL (avl_int_node, int);
TGE_AVL_VERIFY_DEFN (avl_int_verify, avl_int_node);
TGE_AVL_INORDER_DEFN (avl_int_inorder, avl_int_node);
TGE_AVL_SEARCH_DEFN (avl_int_search, avl_int_node, int);
TGE_AVL_INSERT_DEFN (avl_int_insert, avl_int_node, int);
TGE_AVL_DELETE_DEFN (avl_int_delete, avl_int_node, int);

int i;
int j;

void
test_backwards (avl_int_node_t node, void *data)
{
  int *p = (int *) data;
  *p += -1;
  assert (node->data == *p);
}

void
test_forwards (avl_int_node_t node, void *data)
{
  int *p = (int *) data;
  assert (node->data == *p);
  *p += 1;
}

void
the_test (avl_int_node_t *tree)
{
  /* Insert in random order. */
  fisher_yates ();
  *tree = nullptr;
  assert (0 <= (avl_int_verify) (*tree));
  for (i = 0; i != (BUFSIZE); i += 1)
    {
      *tree = (avl_int_insert) (*tree, buf[i], (int_cmp), nullptr);
      assert (0 <= (avl_int_verify) (*tree));
    }

  /* Search in random order. */
  fisher_yates ();
  for (i = 0; i != (BUFSIZE); i += 1)
    assert ((avl_int_search) (*tree, buf[i], (int_cmp), nullptr) !=
            nullptr);

  /* In-order walk backwards. */
  i = BUFSIZE;
  (avl_int_inorder) (*tree, -1, test_backwards, &i);

  /* In-order walk forwards. */
  i = 0;
  (avl_int_inorder) (*tree, 1, test_forwards, &i);

  /* Delete in random order. */
  fisher_yates ();
  for (i = 0; i != (BUFSIZE); i += 1)
    {
      *tree = (avl_int_delete) (*tree, buf[i], (int_cmp), nullptr);
      assert (0 <= (avl_int_verify) (*tree));
      for (j = 0; j != i + 1; j += 1)
        assert ((avl_int_search) (*tree, buf[j], (int_cmp), nullptr) ==
                nullptr);
      for (j = i + 1; j != (BUFSIZE); j += 1)
        assert ((avl_int_search) (*tree, buf[j], (int_cmp), nullptr) !=
                nullptr);
    }
  assert (*tree == nullptr);

  /* Insert in random order. */
  fisher_yates ();
  *tree = nullptr;
  assert (0 <= (avl_int_verify) (*tree));
  for (i = 0; i != (BUFSIZE); i += 1)
    {
      *tree = (avl_int_insert) (*tree, buf[i], (int_cmp), nullptr);
      assert (0 <= (avl_int_verify) (*tree));
    }

  /* Delete in ascending order. */
  fisher_yates ();
  for (i = 0; i != (BUFSIZE); i += 1)
    {
      *tree = (avl_int_delete) (*tree, i, (int_cmp), nullptr);
      assert (0 <= (avl_int_verify) (*tree));
      for (j = 0; j != i + 1; j += 1)
        assert ((avl_int_search) (*tree, j, (int_cmp), nullptr) ==
                nullptr);
      for (j = i + 1; j != (BUFSIZE); j += 1)
        assert ((avl_int_search) (*tree, j, (int_cmp), nullptr) !=
                nullptr);
    }
  assert (*tree == nullptr);
}

int
main (void)
{
  tge_gc_init ();

  TGE_GC_LOCAL_ROOT (avl_int_node_t, tree);
  the_test (&tree);
  TGE_GC_DEREGISTER_ROOT (tree);
}
