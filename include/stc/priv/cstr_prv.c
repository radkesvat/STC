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
#ifndef STC_CSTR_PRV_C_INCLUDED
#define STC_CSTR_PRV_C_INCLUDED

STC_DEF uint64_t cstr_hash(const cstr *self) {
    csview sv = cstr_sv(self);
    return chash_n(sv.buf, sv.size);
}

STC_DEF isize cstr_find_sv(const cstr* self, csview search) {
    csview sv = cstr_sv(self);
    char* res = cstrnstrn(sv.buf, sv.size, search.buf, search.size);
    return res ? (res - sv.buf) : c_NPOS;
}

STC_DEF char* _cstr_internal_move(cstr* self, const isize pos1, const isize pos2) {
    cstr_buf r = cstr_buffer(self);
    if (pos1 != pos2) {
        const isize newlen = (r.size + pos2 - pos1);
        if (newlen > r.cap)
            r.data = cstr_reserve(self, r.size*3/2 + pos2 - pos1);
        c_memmove(&r.data[pos2], &r.data[pos1], r.size - pos1);
        _cstr_set_size(self, newlen);
    }
    return r.data;
}

STC_DEF char* _cstr_init(cstr* self, const isize len, const isize cap) {
    if (cap > cstr_s_cap) {
        self->lon.data = (char *)i_malloc(cap + 1);
        cstr_l_set_size(self, len);
        cstr_l_set_cap(self, cap);
        return self->lon.data;
    }
    cstr_s_set_size(self, len);
    return self->sml.data;
}

STC_DEF char* cstr_reserve(cstr* self, const isize cap) {
    if (cstr_is_long(self)) {
        if (cap > cstr_l_cap(self)) {
            self->lon.data = (char *)i_realloc(self->lon.data, cstr_l_cap(self) + 1, cap + 1);
            cstr_l_set_cap(self, cap);
        }
        return self->lon.data;
    }
    /* from short to long: */
    if (cap > cstr_s_cap) {
        char* data = (char *)i_malloc(cap + 1);
        const isize len = cstr_s_size(self);
        /* copy full short buffer to emulate realloc() */
        c_memcpy(data, self->sml.data, c_sizeof self->sml);
        self->lon.data = data;
        self->lon.size = (size_t)len;
        cstr_l_set_cap(self, cap);
        return data;
    }
    return self->sml.data;
}

STC_DEF char* cstr_resize(cstr* self, const isize size, const char value) {
    cstr_buf r = cstr_buffer(self);
    if (size > r.size) {
        if (size > r.cap && !(r.data = cstr_reserve(self, size)))
            return NULL;
        c_memset(r.data + r.size, value, size - r.size);
    }
    _cstr_set_size(self, size);
    return r.data;
}

STC_DEF isize cstr_find_at(const cstr* self, const isize pos, const char* search) {
    csview sv = cstr_sv(self);
    if (pos > sv.size) return c_NPOS;
    const char* res = strstr((char*)sv.buf + pos, search);
    return res ? (res - sv.buf) : c_NPOS;
}

STC_DEF char* cstr_assign_n(cstr* self, const char* str, const isize len) {
    char* d = cstr_reserve(self, len);
    if (d) { _cstr_set_size(self, len); c_memmove(d, str, len); }
    return d;
}

STC_DEF char* cstr_append_n(cstr* self, const char* str, const isize len) {
    cstr_buf r = cstr_buffer(self);
    if (r.size + len > r.cap) {
        const size_t off = (size_t)(str - r.data);
        r.data = cstr_reserve(self, r.size*3/2 + len);
        if (!r.data) return NULL;
        if (off <= (size_t)r.size) str = r.data + off; /* handle self append */
    }
    c_memcpy(r.data + r.size, str, len);
    _cstr_set_size(self, r.size + len);
    return r.data;
}

STC_DEF char* cstr_append_uninit(cstr *self, isize len) {
    cstr_buf r = cstr_buffer(self);
    if (r.size + len > r.cap && !(r.data = cstr_reserve(self, r.size*3/2 + len)))
        return NULL;
    _cstr_set_size(self, r.size + len);
    return r.data + r.size;
}

STC_DEF cstr cstr_from_replace(csview in, csview search, csview repl, int32_t count) {
    cstr out = cstr_init();
    isize from = 0; char* res;
    if (!count) count = INT32_MAX;
    if (search.size)
        while (count-- && (res = cstrnstrn(in.buf + from, in.size - from, search.buf, search.size))) {
            const isize pos = (res - in.buf);
            cstr_append_n(&out, in.buf + from, pos - from);
            cstr_append_n(&out, repl.buf, repl.size);
            from = pos + search.size;
        }
    cstr_append_n(&out, in.buf + from, in.size - from);
    return out;
}

STC_DEF void cstr_erase(cstr* self, const isize pos, isize len) {
    cstr_buf r = cstr_buffer(self);
    if (len > r.size - pos) len = r.size - pos;
    c_memmove(&r.data[pos], &r.data[pos + len], r.size - (pos + len));
    _cstr_set_size(self, r.size - len);
}

STC_DEF void cstr_u8_erase(cstr* self, const isize bytepos, const isize u8len) {
    cstr_buf r = cstr_buffer(self);
    isize len = utf8_pos(r.data + bytepos, u8len);
    c_memmove(&r.data[bytepos], &r.data[bytepos + len], r.size - (bytepos + len));
    _cstr_set_size(self, r.size - len);
}

#endif // STC_CSTR_PRV_C_INCLUDED

/* ----------------------- UTF8 CASE CONVERSION ---------------------- */
#if !defined STC_CSTR_UTF8_INCLUDED && (defined i_import || defined STC_UTF8_PRV_C_INCLUDED)
#define STC_CSTR_UTF8_INCLUDED

#include <ctype.h>
#include <stdarg.h>

STC_DEF bool cstr_getdelim(cstr *self, const int delim, FILE *fp) {
    int c = fgetc(fp);
    if (c == EOF)
        return false;
    isize pos = 0;
    cstr_buf r = cstr_buffer(self);
    for (;;) {
        if (c == delim || c == EOF) {
            _cstr_set_size(self, pos);
            return true;
        }
        if (pos == r.cap) {
            _cstr_set_size(self, pos);
            r.data = cstr_reserve(self, (r.cap = r.cap*3/2 + 16));
        }
        r.data[pos++] = (char) c;
        c = fgetc(fp);
    }
}

static isize cstr_vfmt(cstr* self, isize start, const char* fmt, va_list args) {
    va_list args2;
    va_copy(args2, args);
    const int n = vsnprintf(NULL, 0ULL, fmt, args);
    vsprintf(cstr_reserve(self, start + n) + start, fmt, args2);
    va_end(args2);
    _cstr_set_size(self, start + n);
    return n;
}

STC_DEF cstr cstr_from_fmt(const char* fmt, ...) {
    cstr s = cstr_init();
    va_list args;
    va_start(args, fmt);
    cstr_vfmt(&s, 0, fmt, args);
    va_end(args);
    return s;
}

STC_DEF isize cstr_append_fmt(cstr* self, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    const isize n = cstr_vfmt(self, cstr_size(self), fmt, args);
    va_end(args);
    return n;
}

/* NB! self-data in args is UB */
STC_DEF isize cstr_printf(cstr* self, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    const isize n = cstr_vfmt(self, 0, fmt, args);
    va_end(args);
    return n;
}

STC_DEF void cstr_shrink_to_fit(cstr* self) {
    cstr_buf r = cstr_buffer(self);
    if (r.size == r.cap)
        return;
    if (r.size > cstr_s_cap) {
        self->lon.data = (char *)i_realloc(self->lon.data, cstr_l_cap(self) + 1, r.size + 1);
        cstr_l_set_cap(self, r.size);
    } else if (r.cap > cstr_s_cap) {
        c_memcpy(self->sml.data, r.data, r.size + 1);
        cstr_s_set_size(self, r.size);
        i_free(r.data, r.cap + 1);
    }
}

// utf8:

STC_DEF bool cstr_u8_valid(const cstr* self)
    { return utf8_valid(cstr_str(self)); }

static struct {
    int      (*conv_asc)(int);
    uint32_t (*conv_utf)(uint32_t);
}
fn_tocase[] = {{tolower, utf8_casefold},
               {tolower, utf8_tolower},
               {toupper, utf8_toupper}};

STC_DEF cstr cstr_tocase_sv(csview sv, int k) {
    cstr out = {0};
    char *buf = cstr_reserve(&out, sv.size*3/2);
    const char *end = sv.buf + sv.size;
    uint32_t cp; isize sz = 0;
    utf8_decode_t d = {.state=0};

    while (sv.buf < end) {
        do { utf8_decode(&d, (uint8_t)*sv.buf++); } while (d.state);
        if (d.codep < 128)
            buf[sz++] = (char)fn_tocase[k].conv_asc((int)d.codep);
        else {
            cp = fn_tocase[k].conv_utf(d.codep);
            sz += utf8_encode(buf + sz, cp);
        }
    }
    _cstr_set_size(&out, sz);
    cstr_shrink_to_fit(&out);
    return out;
}

#endif // i_import STC_CSTR_UTF8_INCLUDED
