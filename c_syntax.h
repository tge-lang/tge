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

#ifndef TGE__C_SYNTAX_H__INCLUDED__
#define TGE__C_SYNTAX_H__INCLUDED__

//----------------------------------------------------------------------
//
// “defer”
//

#if defined(__has_include )
#if __has_include (<stddefer.h>)
#include <stddefer.h>
#endif
#endif

#ifndef defer

#if defined (__has_extension)
#if __has_extension (c_defer)
#define HAVE_NATIVE_DEFER 1
#endif
#elif defined (__has_feature)
#if __has_feature (c_defer)
#define HAVE_NATIVE_DEFER 1
#endif
#endif

#if defined (HAVE_NATIVE_DEFER)
#define defer _Defer
#else
//
// The following implementation of “defer” requires braces. Be sure to
// put a semicolon after the closing brace, because (once “defer” is
// standardized) this will probably be necessary for
// standard-conforming code.
//
#define Defer_A__(N) Defer_B__ (N)
#define Defer_B__(N) Defer_C__ (Defer_func__ ## N, Defer_var__ ## N)
#define Defer_C__(F, V)                               \
        auto void F (int PTR);                                 \
        [[gnu::cleanup(F), maybe_unused]] int V;              \
        inline auto void F ([[maybe_unused]] int PTR V)
#define defer Defer_A__ (__COUNTER__)
#endif

#endif

//----------------------------------------------------------------------
//
// Pointer syntax. Some of this notation is akin to Lisp/Scheme CXR
// notation.
//

//
// A note about CPCTR:
//
// CPCTR, defined below, can be used to create variables that cannot
// be assigned to with = but which can be assigned to by specially
// written functions. Using this method, along with “defer”, one can
// write C code that works to some degree like Ada code that inherits
// from the ada.finalization package.

#define PTR *
#define PPTR **
#define PCTR *const
#define PCPTR *const*
#define PCPCTR *const*const
#define PPCTR **const

#define CPTR const*
#define CPPTR const**
#define CPCTR const*const       // <-- See the note above.
#define CPCPTR const*const*
#define CPCPCTR const*const*const
#define CPPCTR const**const

#define VPTR void*
#define VPPTR void**
#define VPCTR void*const
#define VPCPTR void*const*
#define VPCPCTR void*const*const
#define VPPCTR void**const

#define VCPTR void const*
#define VCPPTR void const**
#define VCPCTR void const*const
#define VCPCPTR void const*const*
#define VCPCPCTR void const*const*const
#define VCPPCTR void const**const

#define PTR_TO &
#define DEREF *
#define PUT *
#define GET *

#define PTR_DIFF(p, q) ((p) - (q))
#define PTR_ADD(p, i) (PTR_TO ((p)[(i)]))
#define PTR_SUB(p, i) (PTR_TO ((p)[-(i)]))

//----------------------------------------------------------------------

#endif // TGE__C_SYNTAX_H__INCLUDED__
