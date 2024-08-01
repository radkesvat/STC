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

/*  Circular Singly-linked Lists.
    This implements a std::forward_list-like class in C. Because it is circular,
    it also support both push_back() and push_front(), unlike std::forward_list:

    #include <stdio.h>
    #include "stc/crand.h"

    #define i_type list_ix,int64_t
    #include "stc/list.h"

    int main(void)
    {
        c_with (list_ix list = {0}, list_ix_drop(&list))
        {
            int n;
            for (int i = 0; i < 1000000; ++i) // one million
                list_ix_push_back(&list, crand() >> 32);
            n = 0;
            c_foreach (i, clist_ix, list)
                if (++n % 10000 == 0) printf("%8d: %10zu\n", n, *i.ref);
            // Sort them...
            list_ix_sort(&list); // qsort O(n*log n)
            n = 0;
            puts("sorted");
            c_foreach (i, list_ix, list)
                if (++n % 10000 == 0) printf("%8d: %10zu\n", n, *i.ref);
        }
    }
*/
#include "priv/linkage.h"

#ifndef STC_LIST_H_INCLUDED
#define STC_LIST_H_INCLUDED
#include "common.h"
#include "types.h"
#include <stdlib.h>
#include <string.h>

#define _c_list_complete_types(SELF, dummy) \
    struct SELF##_node { \
        SELF##_value value; /* must be first! */ \
        struct SELF##_node *next; \
    }

#define _clist_tonode(vp) c_safe_cast(_m_node*, _m_value*, vp)

#define _c_list_insert_entry_after(ref, val) \
    _m_node *entry = _i_malloc(_m_node, 1); entry->value = val; \
    _c_list_insert_after_node(ref, entry)

#define _c_list_insert_after_node(ref, entry) \
    if (ref) entry->next = ref->next, ref->next = entry; \
    else     entry->next = entry
    // +: set self->last based on node

#endif // STC_LIST_H_INCLUDED

#ifndef _i_prefix
  #define _i_prefix list_
#endif
#include "priv/template.h"

#ifndef i_is_forward
  _c_DEFTYPES(_c_list_types, _i_self, i_key);
#endif
_c_DEFTYPES(_c_list_complete_types, _i_self, dummy);
typedef i_keyraw _m_raw;

STC_API void            _c_MEMB(_drop)(const _i_self* cself);
STC_API _m_value*       _c_MEMB(_push_back)(_i_self* self, _m_value value);
STC_API _m_value*       _c_MEMB(_push_front)(_i_self* self, _m_value value);
STC_API _m_iter         _c_MEMB(_insert_at)(_i_self* self, _m_iter it, _m_value value);
STC_API _m_iter         _c_MEMB(_erase_at)(_i_self* self, _m_iter it);
STC_API _m_iter         _c_MEMB(_erase_range)(_i_self* self, _m_iter it1, _m_iter it2);
#if defined _i_has_eq
STC_API _m_iter         _c_MEMB(_find_in)(_m_iter it1, _m_iter it2, _m_raw val);
STC_API intptr_t        _c_MEMB(_remove)(_i_self* self, _m_raw val);
#endif
#if defined _i_has_cmp
STC_API bool            _c_MEMB(_sort_with)(_i_self* self, int(*cmp)(const _m_value*, const _m_value*));
STC_API int             _c_MEMB(_sort_cmp_)(const _m_value*, const _m_value*);
STC_INLINE bool         _c_MEMB(_sort)(_i_self* self)
                            { return _c_MEMB(_sort_with)(self, _c_MEMB(_sort_cmp_)); }
#endif
STC_API void            _c_MEMB(_reverse)(_i_self* self);
STC_API _m_iter         _c_MEMB(_splice)(_i_self* self, _m_iter it, _i_self* other);
STC_API _i_self          _c_MEMB(_split_off)(_i_self* self, _m_iter it1, _m_iter it2);
STC_API _m_value*       _c_MEMB(_push_back_node)(_i_self* self, _m_node* node);
STC_API _m_value*       _c_MEMB(_insert_after_node)(_i_self* self, _m_node* ref, _m_node* node);
STC_API _m_node*        _c_MEMB(_unlink_after_node)(_i_self* self, _m_node* ref);
STC_API void            _c_MEMB(_erase_after_node)(_i_self* self, _m_node* ref);
STC_INLINE _m_node*     _c_MEMB(_get_node)(_m_value* pval) { return _clist_tonode(pval); }
STC_INLINE _m_node*     _c_MEMB(_unlink_front_node)(_i_self* self)
                            { return _c_MEMB(_unlink_after_node)(self, self->last); }
#if !defined i_no_clone
STC_API _i_self          _c_MEMB(_clone)(_i_self cx);
STC_INLINE _m_value     _c_MEMB(_value_clone)(_m_value val) { return i_keyclone(val); }

STC_INLINE void
_c_MEMB(_copy)(_i_self *self, const _i_self* other) {
    if (self->last == other->last) return;
    _c_MEMB(_drop)(self); *self = _c_MEMB(_clone)(*other);
}
#endif // !i_no_clone

#if !defined i_no_emplace
STC_INLINE _m_value*    _c_MEMB(_emplace_back)(_i_self* self, _m_raw raw)
                            { return _c_MEMB(_push_back)(self, i_keyfrom(raw)); }
STC_INLINE _m_value*    _c_MEMB(_emplace_front)(_i_self* self, _m_raw raw)
                            { return _c_MEMB(_push_front)(self, i_keyfrom(raw)); }
STC_INLINE _m_iter      _c_MEMB(_emplace_at)(_i_self* self, _m_iter it, _m_raw raw)
                            { return _c_MEMB(_insert_at)(self, it, i_keyfrom(raw)); }
STC_INLINE _m_value*    _c_MEMB(_emplace)(_i_self* self, _m_raw raw)
                            { return _c_MEMB(_push_back)(self, i_keyfrom(raw)); }
#endif // !i_no_emplace

STC_INLINE _i_self       _c_MEMB(_init)(void) { return c_LITERAL(_i_self){NULL}; }
STC_INLINE void         _c_MEMB(_put_n)(_i_self* self, const _m_raw* raw, intptr_t n)
                            { while (n--) _c_MEMB(_push_back)(self, i_keyfrom(*raw++)); }
STC_INLINE _i_self       _c_MEMB(_from_n)(const _m_raw* raw, intptr_t n)
                            { _i_self cx = {0}; _c_MEMB(_put_n)(&cx, raw, n); return cx; }
STC_INLINE bool         _c_MEMB(_reserve)(_i_self* self, intptr_t n) { (void)(self + n); return true; }
STC_INLINE bool         _c_MEMB(_is_empty)(const _i_self* self) { return self->last == NULL; }
STC_INLINE void         _c_MEMB(_clear)(_i_self* self) { _c_MEMB(_drop)(self); }
STC_INLINE _m_value*    _c_MEMB(_push)(_i_self* self, _m_value value)
                            { return _c_MEMB(_push_back)(self, value); }
STC_INLINE void         _c_MEMB(_pop_front)(_i_self* self)
                            { c_assert(!_c_MEMB(_is_empty)(self)); _c_MEMB(_erase_after_node)(self, self->last); }
STC_INLINE _m_value*    _c_MEMB(_front)(const _i_self* self) { return &self->last->next->value; }
STC_INLINE _m_value*    _c_MEMB(_back)(const _i_self* self) { return &self->last->value; }
STC_INLINE _m_raw       _c_MEMB(_value_toraw)(const _m_value* pval) { return i_keyto(pval); }
STC_INLINE void         _c_MEMB(_value_drop)(_m_value* pval) { i_keydrop(pval); }

STC_INLINE intptr_t
_c_MEMB(_count)(const _i_self* self) {
    intptr_t n = 1; const _m_node *node = self->last;
    if (!node) return 0;
    while ((node = node->next) != self->last) ++n;
    return n;
}

STC_INLINE _m_iter
_c_MEMB(_begin)(const _i_self* self) {
    _m_value* head = self->last ? &self->last->next->value : NULL;
    return c_LITERAL(_m_iter){head, &self->last, self->last};
}

STC_INLINE _m_iter
_c_MEMB(_end)(const _i_self* self)
    { (void)self; return c_LITERAL(_m_iter){NULL}; }

STC_INLINE void
_c_MEMB(_next)(_m_iter* it) {
    _m_node* node = it->prev = _clist_tonode(it->ref);
    it->ref = (node == *it->_last ? NULL : &node->next->value);
}

STC_INLINE _m_iter
_c_MEMB(_advance)(_m_iter it, size_t n) {
    while (n-- && it.ref) _c_MEMB(_next)(&it);
    return it;
}

STC_INLINE _m_iter
_c_MEMB(_splice_range)(_i_self* self, _m_iter it,
                       _i_self* other, _m_iter it1, _m_iter it2) {
    _i_self tmp = _c_MEMB(_split_off)(other, it1, it2);
    return _c_MEMB(_splice)(self, it, &tmp);
}

#if defined _i_has_eq
STC_INLINE _m_iter
_c_MEMB(_find)(const _i_self* self, _m_raw val) {
    return _c_MEMB(_find_in)(_c_MEMB(_begin)(self), _c_MEMB(_end)(self), val);
}

STC_INLINE const _m_value*
_c_MEMB(_get)(const _i_self* self, _m_raw val) {
    return _c_MEMB(_find_in)(_c_MEMB(_begin)(self), _c_MEMB(_end)(self), val).ref;
}

STC_INLINE _m_value*
_c_MEMB(_get_mut)(_i_self* self, _m_raw val) {
    return _c_MEMB(_find_in)(_c_MEMB(_begin)(self), _c_MEMB(_end)(self), val).ref;
}

STC_INLINE bool _c_MEMB(_eq)(const _i_self* self, const _i_self* other) {
    _m_iter i = _c_MEMB(_begin)(self), j = _c_MEMB(_begin)(other);
    for (; i.ref && j.ref; _c_MEMB(_next)(&i), _c_MEMB(_next)(&j)) {
        const _m_raw _rx = i_keyto(i.ref), _ry = i_keyto(j.ref);
        if (!(i_eq((&_rx), (&_ry)))) return false;
    }
    return !(i.ref || j.ref);
}
#endif

// -------------------------- IMPLEMENTATION -------------------------
#if defined(i_implement) || defined(i_static)

#if !defined i_no_clone
STC_DEF _i_self
_c_MEMB(_clone)(_i_self lst) {
    _i_self tmp = {0};
    c_foreach (it, _i_self, lst)
        _c_MEMB(_push_back)(&tmp, i_keyclone((*it.ref)));
    lst.last = tmp.last;
    return lst;
}
#endif

STC_DEF void
_c_MEMB(_drop)(const _i_self* cself) {
    _i_self* self = (_i_self*)cself;
    while (self->last) _c_MEMB(_erase_after_node)(self, self->last);
}

STC_DEF _m_value*
_c_MEMB(_push_back)(_i_self* self, _m_value value) {
    _c_list_insert_entry_after(self->last, value);
    self->last = entry;
    return &entry->value;
}

STC_DEF _m_value*
_c_MEMB(_push_front)(_i_self* self, _m_value value) {
    _c_list_insert_entry_after(self->last, value);
    if (!self->last)
        self->last = entry;
    return &entry->value;
}

STC_DEF _m_value*
_c_MEMB(_push_back_node)(_i_self* self, _m_node* node) {
    _c_list_insert_after_node(self->last, node);
    self->last = node;
    return &node->value;
}

STC_DEF _m_value*
_c_MEMB(_insert_after_node)(_i_self* self, _m_node* ref, _m_node* node) {
    _c_list_insert_after_node(ref, node);
    if (!self->last)
        self->last = node;
    return &node->value;
}

STC_DEF _m_iter
_c_MEMB(_insert_at)(_i_self* self, _m_iter it, _m_value value) {
    _m_node* node = it.ref ? it.prev : self->last;
    _c_list_insert_entry_after(node, value);
    if (!self->last || !it.ref) {
        it.prev = self->last ? self->last : entry;
        self->last = entry;
    }
    it.ref = &entry->value;
    return it;
}

STC_DEF _m_iter
_c_MEMB(_erase_at)(_i_self* self, _m_iter it) {
    _m_node *node = _clist_tonode(it.ref);
    it.ref = (node == self->last) ? NULL : &node->next->value;
    _c_MEMB(_erase_after_node)(self, it.prev);
    return it;
}

STC_DEF _m_iter
_c_MEMB(_erase_range)(_i_self* self, _m_iter it1, _m_iter it2) {
    _m_node *end = it2.ref ? _clist_tonode(it2.ref) : self->last->next;
    if (it1.ref != it2.ref) do {
        _c_MEMB(_erase_after_node)(self, it1.prev);
        if (!self->last) break;
    } while (it1.prev->next != end);
    return it2;
}

STC_DEF void
_c_MEMB(_erase_after_node)(_i_self* self, _m_node* ref) {
    _m_node* node = _c_MEMB(_unlink_after_node)(self, ref);
    i_keydrop((&node->value));
    i_free(node, c_sizeof *node);
}

STC_DEF _m_node*
_c_MEMB(_unlink_after_node)(_i_self* self, _m_node* ref) {
    _m_node* node = ref->next, *next = node->next;
    ref->next = next;
    if (node == next)
        self->last = NULL;
    else if (node == self->last)
        self->last = ref;
    return node;
}

STC_DEF void
_c_MEMB(_reverse)(_i_self* self) {
    _i_self rev = {NULL};
    while (self->last) {
        _m_node* node = _c_MEMB(_unlink_after_node)(self, self->last);
        _c_MEMB(_insert_after_node)(&rev, rev.last, node);
    }
    *self = rev;
}

STC_DEF _m_iter
_c_MEMB(_splice)(_i_self* self, _m_iter it, _i_self* other) {
    if (!self->last)
        self->last = other->last;
    else if (other->last) {
        _m_node *p = it.ref ? it.prev : self->last, *next = p->next;
        it.prev = other->last;
        p->next = it.prev->next;
        it.prev->next = next;
        if (!it.ref) self->last = it.prev;
    }
    other->last = NULL;
    return it;
}

STC_DEF _i_self
_c_MEMB(_split_off)(_i_self* self, _m_iter it1, _m_iter it2) {
    _i_self lst = {NULL};
    if (it1.ref == it2.ref)
        return lst;
    _m_node *p1 = it1.prev,
            *p2 = it2.ref ? it2.prev : self->last;
    p1->next = p2->next;
    p2->next = _clist_tonode(it1.ref);
    if (self->last == p2)
        self->last = (p1 == p2) ? NULL : p1;
    lst.last = p2;
    return lst;
}

#if defined _i_has_eq
STC_DEF _m_iter
_c_MEMB(_find_in)(_m_iter it1, _m_iter it2, _m_raw val) {
    c_foreach (it, _i_self, it1, it2) {
        _m_raw r = i_keyto(it.ref);
        if (i_eq((&r), (&val)))
            return it;
    }
    it2.ref = NULL; return it2;
}

STC_DEF intptr_t
_c_MEMB(_remove)(_i_self* self, _m_raw val) {
    intptr_t n = 0;
    _m_node *prev = self->last, *node;
    if (prev) do {
        node = prev->next;
        _m_raw r = i_keyto((&node->value));
        if (i_eq((&r), (&val))) {
            _c_MEMB(_erase_after_node)(self, prev), ++n;
            if (!self->last) break;
        } else
            prev = node;
    } while (node != self->last);
    return n;
}
#endif

#if defined _i_has_cmp
STC_DEF int _c_MEMB(_sort_cmp_)(const _m_value* x, const _m_value* y) {
    const _m_raw a = i_keyto(x), b = i_keyto(y);
    return i_cmp((&a), (&b));
}

STC_DEF bool _c_MEMB(_sort_with)(_i_self* self, int(*cmp)(const _m_value*, const _m_value*)) {
    intptr_t len = 0, cap = 0;
    _m_value *arr = NULL, *p = NULL;
    c_foreach (i, _i_self, *self) {
        if (len == cap) {
            intptr_t cap_n = cap + cap/2 + 8;
            if (!(p = (_m_value *)i_realloc(arr, cap*c_sizeof *p, cap_n*c_sizeof *p)))
                goto done;
            arr = p, cap = cap_n;
        }
        arr[len++] = *i.ref;
    }
    qsort(arr, (size_t)len, sizeof *arr, (int(*)(const void*, const void*))cmp);
    c_foreach (i, _i_self, *self)
        *i.ref = *p++;
    done: i_free(arr, cap*c_sizeof *arr);
    return p != NULL;
}
#endif // _i_has_cmp
#endif // i_implement
#include "priv/linkage2.h"
#include "priv/template2.h"
