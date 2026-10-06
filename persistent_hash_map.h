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

#ifndef TGE__PERSISTENT_HASH_MAP_H__INCLUDED__
#define TGE__PERSISTENT_HASH_MAP_H__INCLUDED__

/*

  Ideal hash maps.

  The implementation below uses bit-indexing and thus does not require
  population counts nor arrays. The tree is binary. The price paid is
  that there will be more nodes in the tree.

*/

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#ifndef TGE_HASH_MAP_ALLOC
#include <xalloc.h>
#define TGE_HASH_MAP_ALLOC(T) XMALLOC (T)
#endif

typedef enum
{
  tge_hash_map_insert_or_replace = 0,
  tge_hash_map_insert_only = 1,
  tge_hash_map_replace_only = 2
} tge_hash_map_mode_t;

#define TGE_HASH_MAP_NODE_DECL(NAME)            \
  typedef const struct NAME                     \
  {                                             \
    bool is_leaf;                               \
  } *NAME##_t

#define TGE_HASH_MAP_INTERNAL_DECL(NAME)        \
  typedef const struct NAME##_internal          \
  {                                             \
    bool is_leaf;                               \
    const struct NAME *left;                    \
    const struct NAME *right;                   \
  } *NAME##_internal_t

#define TGE_HASH_MAP_LEAF_DECL(NAME, ELEMTYPE)  \
  typedef const struct NAME##_leaf              \
  {                                             \
    bool is_leaf;                               \
    ELEMTYPE element;                           \
  } *NAME##_leaf_t

#define TGE_HASH_MAP_NODES_DECL(NAME, ELEMTYPE) \
  TGE_HASH_MAP_NODE_DECL (NAME);                \
  TGE_HASH_MAP_INTERNAL_DECL (NAME);            \
  TGE_HASH_MAP_LEAF_DECL (NAME, ELEMTYPE)

#define TGE_HASH_MAP_MAKE_INTERNAL(NEW_NODE, NAME, LEFT, RIGHT) \
  do                                                            \
    {                                                           \
      struct NAME##_internal *_NEW_ND__ =                       \
        TGE_HASH_MAP_ALLOC (struct NAME##_internal);            \
      _NEW_ND__->is_leaf = false;                               \
      _NEW_ND__->left = (LEFT);                                 \
      _NEW_ND__->right = (RIGHT);                               \
      NEW_NODE = (struct NAME *) _NEW_ND__;                     \
    }                                                           \
  while (0)

#define TGE_HASH_MAP_MAKE_LEAF(NEW_NODE, NAME, ELEMENT) \
  do                                                    \
    {                                                   \
      struct NAME##_leaf *_NEW_ND__ =                   \
        TGE_HASH_MAP_ALLOC (struct NAME##_leaf);        \
      _NEW_ND__->is_leaf = true;                        \
      _NEW_ND__->element = (ELEMENT);                   \
      NEW_NODE = (struct NAME *) _NEW_ND__;             \
    }                                                   \
  while (0)

#define TGE_HASH_MAP_SEARCH(SOUGHT_NODE, NAME, ELEMTYPE,        \
                            NODE, KEY, CONTEXT, HASHBIT,        \
                            EQUALS)                             \
  do                                                            \
    {                                                           \
      const struct NAME *_SOUGHT_ND__ = (NODE);                 \
      const ELEMTYPE *_KEY__ = (KEY);                           \
      uint64_t _BIT_NUMBER__ = 0;                               \
      while (_SOUGHT_ND__ != nullptr && !_SOUGHT_ND__->is_leaf) \
        {                                                       \
          _SOUGHT_ND__ =                                        \
            (((HASHBIT) (CONTEXT, _BIT_NUMBER__) == 0)          \
             ? ((NAME##_internal_t) _SOUGHT_ND__)->left         \
             : ((NAME##_internal_t) _SOUGHT_ND__)->right);      \
          _BIT_NUMBER__ += 1;                                   \
        }                                                       \
      if (_SOUGHT_ND__ != nullptr)                              \
        if (!((EQUALS)                                          \
              (_KEY__,                                          \
               &((NAME##_leaf_t) _SOUGHT_ND__)->element)))      \
          _SOUGHT_ND__ = nullptr;                               \
      SOUGHT_NODE = (struct NAME##_leaf *) _SOUGHT_ND__;        \
    }                                                           \
  while (0)

/* A walk of the leaf nodes, with callbacks. The order of the walk
   depends on the hash function, among other things, and should be
   considered arbitrary. */
#define TGE_HASH_MAP_WALK_DEFN(FUNC, NAME, ELEMTYPE)            \
  void                                                          \
  FUNC (NAME##_t _Node,                                         \
        void (*_Do_something) (const ELEMTYPE *, void *),       \
        void *_Possibly_some_data)                              \
  {                                                             \
    if (_Node == nullptr)                                       \
      ; /* Do nothing. */                                       \
    else if (_Node->is_leaf)                                    \
      (_Do_something) (&((NAME##_leaf_t) _Node)->element,       \
                       _Possibly_some_data);                    \
    else                                                        \
      {                                                         \
        (FUNC) (((NAME##_internal_t) _Node)->left,              \
                (_Do_something), _Possibly_some_data);          \
        (FUNC) (((NAME##_internal_t) _Node)->right,             \
                (_Do_something), _Possibly_some_data);          \
      }                                                         \
  }

/* Search for a matching element. Return a pointer to it if found,
   nullptr if not found. */
#define TGE_HASH_MAP_SEARCH_DEFN(FUNC, NAME, ELEMTYPE,          \
                                 HASHINIT, HASHBIT, EQUALS)     \
  const ELEMTYPE *                                              \
  FUNC (NAME##_t _Node, const ELEMTYPE *_Key)                   \
  {                                                             \
    void *__context_ = (HASHINIT) (_Key);                       \
    struct NAME##_leaf *__sought_node_;                         \
    TGE_HASH_MAP_SEARCH (__sought_node_, NAME, ELEMTYPE,        \
                         _Node, _Key, __context_,               \
                         (HASHBIT), (EQUALS));                  \
    return ((__sought_node_ == nullptr)                         \
            ? nullptr                                           \
            : &__sought_node_->element);                        \
  }

/* Insert a leaf node, nondestructively. */
#define TGE_HASH_MAP_INSERT_DEFN(FUNC, NAME, ELEMTYPE,                  \
                                 HASHINIT, HASHBIT, EQUALS)             \
                                                                        \
  struct NAME *                                                         \
  FUNC##_55f1d2b8_3cbe_4f5b_91e1_05fb2ce17fd7                           \
  (NAME##_t _Node, const ELEMTYPE *_Element,                            \
   void *_Key_context, unsigned int _Bit_number)                        \
  {                                                                     \
    struct NAME *_result;                                               \
    struct NAME *_nd;                                                   \
    if (_Node == nullptr)                                               \
      /* A new leaf. */                                                 \
      TGE_HASH_MAP_MAKE_LEAF (_result, NAME, *_Element);                \
    else if (_Node->is_leaf)                                            \
      {                                                                 \
        NAME##_leaf_t _Leaf = (NAME##_leaf_t) _Node;                    \
        if ((EQUALS) (_Element, &_Leaf->element))                       \
          /* An equal key, but a new value. */                          \
          TGE_HASH_MAP_MAKE_LEAF (_result, NAME, *_Element);            \
        else                                                            \
          {                                                             \
            /* Branch out. */                                           \
            bool _key_is_left =                                         \
              ((HASHBIT) (_Key_context, _Bit_number) == 0);             \
            void *_leaf_context = (HASHINIT) (&_Leaf->element);         \
            bool _leaf_is_left =                                        \
              ((HASHBIT) (_leaf_context, _Bit_number) == 0);            \
            if (_key_is_left)                                           \
              {                                                         \
                if (_leaf_is_left)                                      \
                  {                                                     \
                    _nd =                                               \
                      (FUNC##_55f1d2b8_3cbe_4f5b_91e1_05fb2ce17fd7)     \
                      (_Node, _Element, _Key_context, _Bit_number + 1); \
                    TGE_HASH_MAP_MAKE_INTERNAL                          \
                      (_result, NAME, _nd, nullptr);                    \
                  }                                                     \
                else                                                    \
                  {                                                     \
                    TGE_HASH_MAP_MAKE_LEAF                              \
                      (_nd, NAME, *_Element);                           \
                    TGE_HASH_MAP_MAKE_INTERNAL                          \
                      (_result, NAME, _nd, _Node);                      \
                  }                                                     \
              }                                                         \
            else                                                        \
              {                                                         \
                if (_leaf_is_left)                                      \
                  {                                                     \
                    TGE_HASH_MAP_MAKE_LEAF                              \
                      (_nd, NAME, *_Element);                           \
                    TGE_HASH_MAP_MAKE_INTERNAL                          \
                      (_result, NAME, _Node, _nd);                      \
                  }                                                     \
                else                                                    \
                  {                                                     \
                    _nd =                                               \
                      (FUNC##_55f1d2b8_3cbe_4f5b_91e1_05fb2ce17fd7)     \
                      (_Node, _Element, _Key_context, _Bit_number + 1); \
                    TGE_HASH_MAP_MAKE_INTERNAL                          \
                      (_result, NAME, nullptr, _nd);                    \
                  }                                                     \
              }                                                         \
          }                                                             \
      }                                                                 \
    else                                                                \
      {                                                                 \
        /* Continue looking for the insertion point. */                 \
        NAME##_internal_t _Internal = (NAME##_internal_t) _Node;        \
        if ((HASHBIT) (_Key_context, _Bit_number) == 0)                 \
          {                                                             \
            _nd = ((FUNC##_55f1d2b8_3cbe_4f5b_91e1_05fb2ce17fd7)        \
                   (_Internal->left, _Element, _Key_context,            \
                    _Bit_number + 1));                                  \
            TGE_HASH_MAP_MAKE_INTERNAL                                  \
              (_result, NAME, _nd, _Internal->right);                   \
          }                                                             \
        else                                                            \
          {                                                             \
            _nd = ((FUNC##_55f1d2b8_3cbe_4f5b_91e1_05fb2ce17fd7)        \
                   (_Internal->right, _Element, _Key_context,           \
                    _Bit_number + 1));                                  \
            TGE_HASH_MAP_MAKE_INTERNAL                                  \
              (_result, NAME, _Internal->left, _nd);                    \
          }                                                             \
      }                                                                 \
    return _result;                                                     \
  }                                                                     \
                                                                        \
  void                                                                  \
  FUNC (NAME##_t _Node, const ELEMTYPE *_Element,                       \
        tge_hash_map_mode_t _Mode,                                      \
        NAME##_t *_Result_node, ssize_t *_Size_change)                  \
  {                                                                     \
    NAME##_leaf_t _leaf;                                                \
    ssize_t _sz_change;                                                 \
    void *_key_context = (HASHINIT) (_Element);                         \
    if (_Mode == tge_hash_map_insert_or_replace                         \
        && _Size_change == nullptr)                                     \
      /* The _sz_change value will not be used. Avoid  */               \
      /* doing a search that will not be needed.       */               \
      _leaf = nullptr;                                                  \
    else                                                                \
      TGE_HASH_MAP_SEARCH (_leaf, NAME, ELEMTYPE,                       \
                           _Node, _Element, _key_context,               \
                           (HASHBIT), (EQUALS));                        \
    bool _do_insertion;                                                 \
    switch (_Mode)                                                      \
      {                                                                 \
      case tge_hash_map_insert_or_replace:                              \
        _do_insertion = true;                                           \
        _sz_change = (_leaf == nullptr) ? 1 : 0;                        \
        break;                                                          \
      case tge_hash_map_insert_only:                                    \
        _do_insertion = (_leaf == nullptr);                             \
        _sz_change = (_leaf == nullptr) ? 1 : 0;                        \
        break;                                                          \
      case tge_hash_map_replace_only:                                   \
        _do_insertion = (_leaf != nullptr);                             \
        _sz_change = 0;                                                 \
        break;                                                          \
      default:                                                          \
        assert (0);                                                     \
        abort ();                                                       \
      }                                                                 \
    if (_Size_change != nullptr)                                        \
      *_Size_change = _sz_change;                                       \
    if (_Result_node != nullptr)                                        \
      {                                                                 \
        if (_do_insertion)                                              \
          *_Result_node =                                               \
            (FUNC##_55f1d2b8_3cbe_4f5b_91e1_05fb2ce17fd7)               \
            (_Node, _Element, _key_context, 0);                         \
        else                                                            \
          *_Result_node = _Node;                                        \
      }                                                                 \
  }

/* Delete a leaf node, nondestructively. */
#define TGE_HASH_MAP_DELETE_DEFN(FUNC, NAME, ELEMTYPE,                  \
                                 HASHINIT, HASHBIT, EQUALS)             \
                                                                        \
  const struct NAME *                                                   \
  FUNC##_49436463_853f_4e2e_8c23_97e67636e7d8                           \
  (NAME##_t _Node, const ELEMTYPE *_Key,                                \
   void *_Key_context, unsigned int _Bit_number)                        \
  {                                                                     \
    assert (_Node != nullptr);                                          \
    const struct NAME *_nd;                                             \
    const struct NAME *_result = nullptr;                               \
    if (!_Node->is_leaf)                                                \
      {                                                                 \
        NAME##_internal_t _Internal = (NAME##_internal_t) _Node;        \
        if ((HASHBIT) (_Key_context, _Bit_number) == 0)                 \
          {                                                             \
            _nd = ((FUNC##_49436463_853f_4e2e_8c23_97e67636e7d8)        \
                   (_Internal->left, _Key,                              \
                    _Key_context, _Bit_number + 1));                    \
            if (_nd != nullptr                                          \
                && _nd->is_leaf                                         \
                && _Internal->right == nullptr)                         \
              _result = _nd;                                            \
            else if (_nd == nullptr &&                                  \
                     _Internal->right != nullptr                        \
                     && _Internal->right->is_leaf)                      \
              _result = _Internal->right;                               \
            else                                                        \
              TGE_HASH_MAP_MAKE_INTERNAL                                \
                (_result, NAME, _nd, _Internal->right);                 \
          }                                                             \
        else                                                            \
          {                                                             \
            _nd = ((FUNC##_49436463_853f_4e2e_8c23_97e67636e7d8)        \
                   (_Internal->right, _Key,                             \
                    _Key_context, _Bit_number + 1));                    \
            if (_nd != nullptr                                          \
                && _nd->is_leaf                                         \
                && _Internal->left == nullptr)                          \
              _result = _nd;                                            \
            else if (_nd == nullptr                                     \
                     && _Internal->left != nullptr                      \
                     && _Internal->left->is_leaf)                       \
              _result = _Internal->left;                                \
            else                                                        \
              TGE_HASH_MAP_MAKE_INTERNAL                                \
                (_result, NAME, _Internal->left, _nd);                  \
          }                                                             \
      }                                                                 \
    return _result;                                                     \
  }                                                                     \
                                                                        \
  void                                                                  \
  FUNC (NAME##_t _Node, const ELEMTYPE *_Key,                           \
        NAME##_t *_Result_node, ssize_t *_Size_change)                  \
  {                                                                     \
    const struct NAME *_result = _Node;                                 \
    struct NAME##_leaf *_leaf;                                          \
    void *_key_context = (HASHINIT) (_Key);                             \
    TGE_HASH_MAP_SEARCH (_leaf, NAME, ELEMTYPE,                         \
                         _Node, _Key, _key_context,                     \
                         (HASHBIT), (EQUALS));                          \
    if (_leaf != nullptr)                                               \
      _result =                                                         \
        (FUNC##_49436463_853f_4e2e_8c23_97e67636e7d8)                   \
        (_Node, _Key, _key_context, 0);                                 \
    if (_Result_node != nullptr)                                        \
      *_Result_node = _result;                                          \
    if (_Size_change != nullptr)                                        \
      *_Size_change = (_leaf != nullptr) ? -1 : 0;                      \
  }

#endif /* TGE__PERSISTENT_HASH_MAP_H__INCLUDED__ */

/*
  local variables:
  mode: c
  coding: utf-8
  end:
*/
