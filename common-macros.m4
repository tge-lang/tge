m4_divert(-1)
|-----------------------------------------------------------------------
| SPDX-License-Identifier: MIT
|-----------------------------------------------------------------------

m4_changequote([:,:])
m4_changecom

m4_dnl
m4_dnl  m4_foreachq(x, [:item_1, item_2, ..., item_n:], statement)
m4_dnl
m4_define([:m4_foreachq:],
  [:m4_ifelse([:$2:],[::],[::],
    [:m4_pushdef([:$1:])_$0([:$1:],[:$3:],[::],$2)m4_popdef([:$1:]):]):])
m4_define([:_m4_foreachq:],
  [:m4_ifelse([:$#:],[:3:],[::],
    [:m4_define([:$1:],[:$4:])$2[::]$0([:$1:],[:$2:],m4_shift(m4_shift(m4_shift($@)))):]):])

m4_dnl
m4_dnl  m4_forloop(variable, from, to, statement)
m4_dnl
m4_define([:m4_forloop:],[:m4_ifelse(m4_eval([:($2) <= ($3):]),[:1:],
  [:m4_pushdef([:$1:])_$0([:$1:],m4_eval([:$2:]),
    m4_eval([:$3:]),[:$4:])m4_popdef([:$1:]):]):])
m4_define([:_m4_forloop:],
  [:m4_define([:$1:],[:$2:])$4[::]m4_ifelse([:$2:],[:$3:],[::],
    [:$0([:$1:],m4_incr([:$2:]),[:$3:],[:$4:]):]):])

m4_define([:m4_newline:],[:
:])
m4_define([:m4_nonewline:],[:m4_dnl:])
m4_define([:m4_lparen:],[:(:])
m4_define([:m4_rparen:],[:):])
m4_define([:m4_sharpsign:],[:#:])

m4_divert[::]m4_dnl
