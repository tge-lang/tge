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

#ifndef __TGE__PERSISTENT_AVL_H__INCLUDED__
#define __TGE__PERSISTENT_AVL_H__INCLUDED__

#include <stdlib.h>
#include <assert.h>

#ifndef TGE_AVL_ALLOC
#include <tge_gc.h>
#define TGE_AVL_ALLOC(T) ((tge_gc_malloc) (sizeof (T)))
//#include <tge_xalloc.h>
//#define TGE_AVL_ALLOC(T) (tge_xmalloc (sizeof (T)))
#endif

#define TGE_AVL_HEIGHT_T unsigned short int
#define TGE_AVL_NODE_DECL(NAME, T)              \
  typedef struct NAME                           \
  {                                             \
    T data;                                     \
    TGE_AVL_HEIGHT_T height;                    \
    struct NAME *left;                          \
    struct NAME *right;                         \
  } *NAME##_t;

#define TGE_AVL_NODE_BALANCE(NAME, NODE)                                \
  (((NODE) == nullptr) ? ((int) 0) :                                    \
   (((int) ((NODE)->left == nullptr) ? 0 : (NODE)->left->height)        \
    - ((int) ((NODE)->right == nullptr) ? 0 : (NODE)->right->height)))

#define TGE_AVL_MAKE_NODE(NEW_NODE, NAME, DATA, LEFT, RIGHT)            \
  do                                                                    \
    {                                                                   \
      struct NAME *_node__ = TGE_AVL_ALLOC (struct NAME);               \
      _node__->data = (DATA);                                           \
      _node__->left = (LEFT);                                           \
      _node__->right = (RIGHT);                                         \
      const TGE_AVL_HEIGHT_T _Lheight =                                 \
        (_node__->left == nullptr) ? 0 : _node__->left->height;         \
      const TGE_AVL_HEIGHT_T _Rheight =                                 \
        (_node__->right == nullptr) ? 0 : _node__->right->height;       \
      _node__->height = /* (the greater of the two heights) + 1 */      \
        ((_Lheight < _Rheight) ? _Rheight : _Lheight) + 1;              \
      NEW_NODE = _node__;                                               \
    }                                                                   \
  while (0)

/* Nondestructive rotation. */
#define TGE_AVL_ROTATE_LEFT(NEW_NODE, NAME, NODE)       \
  do                                                    \
    {                                                   \
      struct NAME *_nd__ = (NODE);                      \
      struct NAME *_nd_R__ = _nd__->right;              \
      struct NAME *_new__;                              \
      TGE_AVL_MAKE_NODE (_new__, NAME, _nd__->data,     \
                         _nd__->left, _nd_R__->left);   \
      TGE_AVL_MAKE_NODE (_new__, NAME, _nd_R__->data,   \
                         _new__, _nd_R__->right);       \
      NEW_NODE = _new__;                                \
    }                                                   \
  while (0)

/* Nondestructive rotation. */
#define TGE_AVL_ROTATE_RIGHT(NEW_NODE, NAME, NODE)      \
  do                                                    \
    {                                                   \
      struct NAME *_nd__ = (NODE);                      \
      struct NAME *_nd_L__ = _nd__->left;               \
      struct NAME *_new__;                              \
      TGE_AVL_MAKE_NODE (_new__, NAME, _nd__->data,     \
                         _nd_L__->right, _nd__->right); \
      TGE_AVL_MAKE_NODE (_new__, NAME, _nd_L__->data,   \
                         _nd_L__->left, _new__);        \
      NEW_NODE = _new__;                                \
    }                                                   \
  while (0)

/* Verify that a tree is balanced. Return the height if the tree is
   balanced, or -1 if it is not. */
#define TGE_AVL_VERIFY_DEFN(FUNC, NAME)                 \
  int                                                   \
  FUNC (struct NAME *_Node)                             \
  {                                                     \
    int result = -1;                                    \
    if (_Node == nullptr)                               \
      result = 0;                                       \
    else                                                \
      {                                                 \
        int height_L = (FUNC) (_Node->left);            \
        if (height_L != -1)                             \
          {                                             \
            int height_R = (FUNC) (_Node->right);       \
            if (height_R != -1)                         \
              {                                         \
                int diff = height_L - height_R;         \
                if (diff == -1 || diff == 0)            \
                  result = height_R + 1;                \
                else if (diff == 1)                     \
                  result = height_L + 1;                \
              }                                         \
          }                                             \
      }                                                 \
    return result;                                      \
  }

/* An in-order walk with callbacks. */
#define TGE_AVL_INORDER_DEFN(FUNC, NAME)                        \
  void                                                          \
  FUNC (struct NAME *_Node, int _Direction,                     \
        void (*_Do_something) (struct NAME *, void *),          \
        void *_Possibly_some_data)                              \
  {                                                             \
    assert (_Direction == -1 || _Direction == 1);               \
    if (_Node != nullptr)                                       \
      {                                                         \
        if (_Direction == -1)                                   \
          {                                                     \
            (FUNC) (_Node->right, _Direction, (_Do_something),  \
                    _Possibly_some_data);                       \
            (_Do_something) (_Node, _Possibly_some_data);       \
            (FUNC) (_Node->left, _Direction, (_Do_something),   \
                    _Possibly_some_data);                       \
          }                                                     \
        else                                                    \
          {                                                     \
            (FUNC) (_Node->left, _Direction, (_Do_something),   \
                    _Possibly_some_data);                       \
            (_Do_something) (_Node, _Possibly_some_data);       \
            (FUNC) (_Node->right, _Direction, (_Do_something),  \
                    _Possibly_some_data);                       \
          }                                                     \
      }                                                         \
  }

/* Search for a matching node. */
#define TGE_AVL_SEARCH_DEFN(FUNC, NAME, T)                      \
  struct NAME *                                                 \
  FUNC (struct NAME *_Node, T _Data,                            \
        int (*_Compare) (const T *, const T *, void *),         \
        void *_Extra)                                           \
  {                                                             \
    struct NAME *_Result = nullptr;                             \
    if (_Node != nullptr)                                       \
      {                                                         \
        int _Cmp = (_Compare) (&_Data, &_Node->data, _Extra);   \
        if (_Cmp < 0)                                           \
          _Result =                                             \
            (FUNC) (_Node->left, _Data, (_Compare), _Extra);    \
        else if (_Cmp == 0)                                     \
          _Result = _Node;                                      \
        else                                                    \
          _Result =                                             \
            (FUNC) (_Node->right, _Data, (_Compare), _Extra);   \
      }                                                         \
    return _Result;                                             \
  }

/* Insert a node, nondestructively. */
#define TGE_AVL_INSERT_DEFN(FUNC, NAME, T)                              \
  struct NAME *                                                         \
  FUNC (struct NAME *_Node, T _Data,                                    \
        int (*_Compare) (const T *, const T *, void *),                 \
        void *_Extra)                                                   \
  {                                                                     \
    struct NAME *_Result;                                               \
    if (_Node == nullptr)                                               \
      TGE_AVL_MAKE_NODE (_Result, NAME, _Data, nullptr, nullptr);       \
    else                                                                \
      {                                                                 \
        int _Cmp = (_Compare) (&_Data, &_Node->data, _Extra);           \
        if (_Cmp == 0)                                                  \
          /* Same key, different value. */                              \
          TGE_AVL_MAKE_NODE (_Result, NAME, _Data, _Node->left,         \
                             _Node->right);                             \
        else                                                            \
          {                                                             \
            if (_Cmp < 0)                                               \
              TGE_AVL_MAKE_NODE (_Result, NAME, _Node->data,            \
                                 (FUNC) (_Node->left, _Data,            \
                                         (_Compare), _Extra),           \
                                 _Node->right);                         \
            else                                                        \
              TGE_AVL_MAKE_NODE (_Result, NAME, _Node->data,            \
                                 _Node->left,                           \
                                 (FUNC) (_Node->right, _Data,           \
                                         (_Compare), _Extra));          \
                                                                        \
            /* Rebalance. */                                            \
            int _Bal = TGE_AVL_NODE_BALANCE (NAME, _Result);            \
            if (_Bal < -1)                                              \
              {                                                         \
                _Cmp =                                                  \
                  (_Compare) (&_Data, &_Result->right->data, _Extra);   \
                if (_Cmp < 0)                                           \
                  {                                                     \
                    struct NAME *_nd;                                   \
                    TGE_AVL_ROTATE_RIGHT (_nd, NAME, _Result->right);   \
                    TGE_AVL_MAKE_NODE (_nd, NAME, _Result->data,        \
                                       _Result->left, _nd);             \
                    TGE_AVL_ROTATE_LEFT (_Result, NAME, _nd);           \
                  }                                                     \
                else if (0 < _Cmp)                                      \
                  TGE_AVL_ROTATE_LEFT (_Result, NAME, _Result);         \
              }                                                         \
            else if (1 < _Bal)                                          \
              {                                                         \
                _Cmp =                                                  \
                  (_Compare) (&_Data, &_Result->left->data, _Extra);    \
                if (_Cmp < 0)                                           \
                  TGE_AVL_ROTATE_RIGHT (_Result, NAME, _Result);        \
                else if (0 < _Cmp)                                      \
                  {                                                     \
                    struct NAME *_nd;                                   \
                    TGE_AVL_ROTATE_LEFT (_nd, NAME, _Result->left);     \
                    TGE_AVL_MAKE_NODE (_nd, NAME, _Result->data,        \
                                       _nd, _Result->right);            \
                    TGE_AVL_ROTATE_RIGHT (_Result, NAME, _nd);          \
                  }                                                     \
              }                                                         \
          }                                                             \
      }                                                                 \
    return _Result;                                                     \
  }

/* Delete a node, nondestructively. */
#define TGE_AVL_DELETE_DEFN(FUNC, NAME, T)                              \
  struct NAME *                                                         \
  FUNC (struct NAME *_Node, T _Data,                                    \
        int (*_Compare) (const T *, const T *, void *),                 \
        void *_Extra)                                                   \
  {                                                                     \
    struct NAME *_Result = nullptr;                                     \
    if (_Node != nullptr)                                               \
      {                                                                 \
        bool needs_rebalancing = false;                                 \
        int _Cmp = (_Compare) (&_Data, &_Node->data, _Extra);           \
        if (_Cmp < 0)                                                   \
          {                                                             \
            TGE_AVL_MAKE_NODE (_Result, NAME, _Node->data,              \
                               (FUNC) (_Node->left, _Data,              \
                                       (_Compare), _Extra),             \
                               _Node->right);                           \
            needs_rebalancing = true;                                   \
          }                                                             \
        else if (0 < _Cmp)                                              \
          {                                                             \
            TGE_AVL_MAKE_NODE (_Result, NAME, _Node->data,              \
                               _Node->left,                             \
                               (FUNC) (_Node->right, _Data,             \
                                       (_Compare), _Extra));            \
            needs_rebalancing = true;                                   \
          }                                                             \
        else                                                            \
          {                                                             \
            /* _Node is the node to be deleted.                    */   \
            if (_Node->left == nullptr)                                 \
              /* _Node is a leaf or _Node->right is a leaf.        */   \
              /* The result will be nullptr or a leaf.             */   \
              _Result = _Node->right;                                   \
            else if (_Node->right == nullptr)                           \
              /* _Node->left is a leaf. The result will be a leaf. */   \
              _Result = _Node->left;                                    \
            else                                                        \
              {                                                         \
                /* Find the in-order successor to _Node.           */   \
                struct NAME *_succ = _Node->right;                      \
                while (_succ->left != nullptr)                          \
                  _succ = _succ->left;                                  \
                                                                        \
                /* Move the successor to where _Node is.           */   \
                TGE_AVL_MAKE_NODE (_Result, NAME, _succ->data,          \
                                   _Node->left,                         \
                                   (FUNC) (_Node->right, _succ->data,   \
                                           (_Compare), _Extra));        \
                needs_rebalancing = true;                               \
              }                                                         \
          }                                                             \
        if (_Result != nullptr && needs_rebalancing)                    \
          {                                                             \
            int _Bal = TGE_AVL_NODE_BALANCE (NAME, _Result);            \
            if (_Bal < -1)                                              \
              {                                                         \
                int _BalR =                                             \
                  TGE_AVL_NODE_BALANCE (NAME, _Result->right);          \
                if (_BalR <= 0)                                         \
                  TGE_AVL_ROTATE_LEFT (_Result, NAME, _Result);         \
                else                                                    \
                  {                                                     \
                    struct NAME *_nd;                                   \
                    TGE_AVL_ROTATE_RIGHT (_nd, NAME, _Result->right);   \
                    TGE_AVL_MAKE_NODE (_nd, NAME, _Result->data,        \
                                       _Result->left, _nd);             \
                    TGE_AVL_ROTATE_LEFT (_Result, NAME, _nd);           \
                  }                                                     \
              }                                                         \
            else if (1 < _Bal)                                          \
              {                                                         \
                int _BalL =                                             \
                  TGE_AVL_NODE_BALANCE (NAME, _Result->left);           \
                if (0 <= _BalL)                                         \
                  TGE_AVL_ROTATE_RIGHT (_Result, NAME, _Result);        \
                else                                                    \
                  {                                                     \
                    struct NAME *_nd;                                   \
                    TGE_AVL_ROTATE_LEFT (_nd, NAME, _Result->left);     \
                    TGE_AVL_MAKE_NODE (_nd, NAME, _Result->data,        \
                                       _nd, _Result->right);            \
                    TGE_AVL_ROTATE_RIGHT (_Result, NAME, _nd);          \
                  }                                                     \
              }                                                         \
          }                                                             \
      }                                                                 \
    return _Result;                                                     \
  }

#endif /* __TGE__PERSISTENT_AVL_H__INCLUDED__ */
