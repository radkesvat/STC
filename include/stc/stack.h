/* MIT License
 *
 * Copyright (c) 2024 Tyge Løvset
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "priv/linkage.h"

#ifndef STC_STACK_H_INCLUDED
#define STC_STACK_H_INCLUDED
#include "common.h"
#include "types.h"
#include <stdlib.h>
#endif // STC_STACK_H_INCLUDED

#ifndef _i_prefix
  #define _i_prefix stack_
#endif
#include "priv/template.h"

#ifndef i_is_forward
#ifdef i_capacity
  #define i_no_clone
  _c_DEFTYPES(_c_stack_fixed, _i_self, i_key, i_capacity);
#else
  _c_DEFTYPES(_c_stack_types, _i_self, i_key);
#endif
#endif
typedef i_keyraw _m_raw;

#ifdef i_capacity
STC_INLINE void _c_MEMB(_init)(_i_self* self)
    { self->_len = 0; }
#else
STC_INLINE _i_self _c_MEMB(_init)(void) {
    _i_self out = {0};
    return out;
}

STC_INLINE _i_self _c_MEMB(_with_capacity)(intptr_t cap) {
    _i_self out = {_i_malloc(_m_value, cap), 0, cap};
    return out;
}

STC_INLINE _i_self _c_MEMB(_with_size)(intptr_t size, _m_value null) {
    _i_self out = {_i_malloc(_m_value, size), size, size};
    while (size) out.data[--size] = null;
    return out;
}
#endif // i_capacity

STC_INLINE void _c_MEMB(_clear)(_i_self* self) {
    _m_value *p = self->data + self->_len;
    while (p-- != self->data) { i_keydrop(p); }
    self->_len = 0;
}

STC_INLINE void _c_MEMB(_drop)(const _i_self* cself) {
    _i_self* self = (_i_self*)cself;
    _c_MEMB(_clear)(self);
#ifndef i_capacity
    i_free(self->data, self->_cap*c_sizeof(*self->data));
#endif
}

STC_INLINE intptr_t _c_MEMB(_size)(const _i_self* self)
    { return self->_len; }

STC_INLINE bool _c_MEMB(_is_empty)(const _i_self* self)
    { return !self->_len; }

STC_INLINE intptr_t _c_MEMB(_capacity)(const _i_self* self) {
#ifndef i_capacity
    return self->_cap;
#else
    (void)self; return i_capacity;
#endif
}

STC_INLINE void _c_MEMB(_value_drop)(_m_value* val)
    { i_keydrop(val); }

STC_INLINE bool _c_MEMB(_reserve)(_i_self* self, intptr_t n) {
    if (n < self->_len) return true;
#ifndef i_capacity
    _m_value *d = (_m_value *)i_realloc(self->data, self->_cap*c_sizeof *d, n*c_sizeof *d);
    if (d) { self->_cap = n, self->data = d; return true; }
#endif
    return false;
}

STC_INLINE _m_value* _c_MEMB(_append_uninit)(_i_self *self, intptr_t n) {
    intptr_t len = self->_len;
    if (len + n > _c_MEMB(_capacity)(self))
        if (!_c_MEMB(_reserve)(self, len*3/2 + n))
            return NULL;
    self->_len += n;
    return self->data + len;
}

STC_INLINE void _c_MEMB(_shrink_to_fit)(_i_self* self)
    { _c_MEMB(_reserve)(self, self->_len); }

STC_INLINE const _m_value* _c_MEMB(_top)(const _i_self* self)
    { return &self->data[self->_len - 1]; }

STC_INLINE _m_value* _c_MEMB(_back)(const _i_self* self)
    { return (_m_value*) &self->data[self->_len - 1]; }

STC_INLINE _m_value* _c_MEMB(_front)(const _i_self* self)
    { return (_m_value*) &self->data[0]; }

STC_INLINE _m_value* _c_MEMB(_push)(_i_self* self, _m_value val) {
    if (self->_len == _c_MEMB(_capacity)(self))
        if (!_c_MEMB(_reserve)(self, self->_len*3/2 + 4))
            return NULL;
    _m_value* vp = self->data + self->_len++;
    *vp = val; return vp;
}

STC_INLINE void _c_MEMB(_pop)(_i_self* self)
    { c_assert(self->_len); _m_value* p = &self->data[--self->_len]; i_keydrop(p); }

STC_INLINE _m_value _c_MEMB(_pull)(_i_self* self)
    { c_assert(self->_len); return self->data[--self->_len]; }

STC_INLINE void _c_MEMB(_put_n)(_i_self* self, const _m_raw* raw, intptr_t n)
    { while (n--) _c_MEMB(_push)(self, i_keyfrom((*raw))), ++raw; }

STC_INLINE _i_self _c_MEMB(_from_n)(const _m_raw* raw, intptr_t n)
    { _i_self cx = {0}; _c_MEMB(_put_n)(&cx, raw, n); return cx; }

STC_INLINE const _m_value* _c_MEMB(_at)(const _i_self* self, intptr_t idx)
    { c_assert(idx < self->_len); return self->data + idx; }

STC_INLINE _m_value* _c_MEMB(_at_mut)(_i_self* self, intptr_t idx)
    { c_assert(idx < self->_len); return self->data + idx; }

#if !defined i_no_emplace
STC_INLINE _m_value* _c_MEMB(_emplace)(_i_self* self, _m_raw raw)
    { return _c_MEMB(_push)(self, i_keyfrom(raw)); }
#endif // !i_no_emplace

#if !defined i_no_clone
STC_INLINE _i_self _c_MEMB(_clone)(_i_self s) {
    _i_self tmp = {_i_malloc(_m_value, s._len), s._len, s._len};
    if (!tmp.data) tmp._cap = 0;
    else for (intptr_t i = 0; i < s._len; ++s.data)
        tmp.data[i++] = i_keyclone((*s.data));
    s.data = tmp.data;
    s._cap = tmp._cap;
    return s;
}

STC_INLINE void _c_MEMB(_copy)(_i_self *self, const _i_self* other) {
    if (self->data == other->data) return;
    _c_MEMB(_drop)(self);
    *self = _c_MEMB(_clone)(*other);
}

STC_INLINE _m_value _c_MEMB(_value_clone)(_m_value val)
    { return i_keyclone(val); }

STC_INLINE i_keyraw _c_MEMB(_value_toraw)(const _m_value* val)
    { return i_keyto(val); }
#endif // !i_no_clone

STC_INLINE _m_iter _c_MEMB(_begin)(const _i_self* self) {
    intptr_t n = self->_len; _m_value* d = (_m_value*)self->data;
    return c_LITERAL(_m_iter){n ? d : NULL, d + n};
}

STC_INLINE _m_iter _c_MEMB(_rbegin)(const _i_self* self) {
    intptr_t n = self->_len; _m_value* d = (_m_value*)self->data;
    return c_LITERAL(_m_iter){n ? d + n - 1 : NULL, d - 1};
}

STC_INLINE _m_iter _c_MEMB(_end)(const _i_self* self)
    { (void)self; return c_LITERAL(_m_iter){0}; }

STC_INLINE _m_iter _c_MEMB(_rend)(const _i_self* self)
    { (void)self; return c_LITERAL(_m_iter){0}; }

STC_INLINE void _c_MEMB(_next)(_m_iter* it)
    { if (++it->ref == it->end) it->ref = NULL; }

STC_INLINE void _c_MEMB(_rnext)(_m_iter* it)
    { if (--it->ref == it->end) it->ref = NULL; }

STC_INLINE _m_iter _c_MEMB(_advance)(_m_iter it, size_t n)
    { if ((it.ref += n) >= it.end) it.ref = NULL ; return it; }

STC_INLINE intptr_t _c_MEMB(_index)(const _i_self* self, _m_iter it)
    { return (it.ref - self->data); }

STC_INLINE void _c_MEMB(_adjust_end_)(_i_self* self, intptr_t n)
    { self->_len += n; }

#if defined _i_has_eq
STC_INLINE bool
_c_MEMB(_eq)(const _i_self* self, const _i_self* other) {
    if (self->_len != other->_len) return false;
    for (intptr_t i = 0; i < self->_len; ++i) {
        const _m_raw _rx = i_keyto(self->data+i), _ry = i_keyto(other->data+i);
        if (!(i_eq((&_rx), (&_ry)))) return false;
    }
    return true;
}
#endif
#include "priv/linkage2.h"
#include "priv/template2.h"
