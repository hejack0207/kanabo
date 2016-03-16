/*
 * 金棒 (kanabō)
 * Copyright (c) 2012 Kevin Birch <kmb@pobox.com>.  All rights reserved.
 * 
 * 金棒 is a tool to bludgeon YAML and JSON files from the shell: the strong
 * made stronger.
 *
 * For more information, consult the README file in the project root.
 *
 * Distributed under an [MIT-style][license] license.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal with
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimers.
 * - Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimers in the documentation and/or
 *   other materials provided with the distribution.
 * - Neither the names of the copyright holders, nor the names of the authors, nor
 *   the names of other contributors may be used to endorse or promote products
 *   derived from this Software without specific prior written permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE CONTRIBUTORS
 * OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
 *
 * [license]: http://www.opensource.org/licenses/ncsa
 */


#include <stdlib.h>
#include <string.h>

#include "str.h"


struct string_s
{
    size_t  length;
    uint8_t value[];
};

struct mutable_string_s
{
    size_t  capacity;
    String  base;
};


static inline String *string_alloc(size_t length)
{
    return calloc(1, sizeof(String) + length + 1);
}

static inline String *string_init(String *self, const char *value, size_t length)
{
    self->length = length;
    memcpy(self->value, value, length);
    self->value[self->length] = '\0';
    return self;
}

String *make_string(const char *value)
{
    size_t length = strlen(value);
    String *self = string_alloc(length);
    if(NULL == self)
    {
        return NULL;
    }
    return string_init(self, value, length);
}

void string_free(String *self)
{
    free(self);
}

String *string_clone(const String *self)
{
    String *that = string_alloc(self->length);
    if(NULL == that)
    {
        return NULL;
    }
    return string_init(that, (const char *)self->value, self->length);
}

size_t string_get_length(String *self)
{
    return self->length;
}

uint8_t string_get_char(const String *self, size_t index)
{
    return self->value[index];
}

const char *string_as_c_str(String *self)
{
    return (const char *)self->value;
}

static inline size_t calculate_allocation_size(size_t capacity)
{
    return sizeof(MutableString) + capacity + 1;
}

static inline MutableString *mstring_alloc(size_t capacity)
{
    return calloc(1, calculate_allocation_size(capacity));
}

static inline bool mstring_realloc(MutableString **self, size_t capacity)
{
    MutableString *cache = *self;
    *self = realloc(*self, calculate_allocation_size(capacity));
    if(NULL == *self)
    {
        *self = cache;
        return false;
    }
    (*self)->capacity = capacity;
    return true;
}

static inline MutableString *mstring_init(MutableString *self, size_t capacity)
{
    self->base.length = 0;
    self->capacity = capacity;
    return self;
}

MutableString *make_mstring(size_t capacity)
{
    MutableString *self = mstring_alloc(capacity);
    if(NULL == self)
    {
        return NULL;
    }
    return mstring_init(self, capacity);
}

MutableString *make_mstring_with_char(const uint8_t value)
{
    MutableString *self = make_mstring(1);
    if(NULL == self)
    {
        return NULL;
    }
    mstring_append(&self, value);
    return self;
}

MutableString *make_mstring_with_c_str(const char *value)
{
    size_t length = strlen(value);
    MutableString *self = make_mstring(length);
    if(NULL == self)
    {
        return NULL;
    }
    mstring_append(&self, value);
    return self;
}

MutableString *make_mstring_with_string(const String *value)
{
    MutableString *self = make_mstring(value->length);
    if(NULL == self)
    {
        return NULL;
    }
    mstring_append(&self, value);
    return self;
}

void mstring_free(MutableString *self)
{
    free(self);
}

size_t mstring_get_length(MutableString *self)
{
    return self->base.length;
}

uint8_t mstring_get_char(const MutableString *self, size_t index)
{
    return self->base.value[index];
}

size_t mstring_get_capacity(MutableString *self)
{
    return self->capacity;
}

bool mstring_has_capacity(MutableString *self, size_t length)
{
    return (self->capacity - self->base.length) >= length;
}

MutableString *mstring_clone(MutableString *self)
{
    MutableString *that = mstring_alloc(self->capacity);
    if(NULL == that)
    {
        return NULL;
    }
    mstring_init(that, self->capacity);
    if(self->base.length)
    {
        memcpy(that->base.value, self->base.value, self->capacity + 1);
    }
    return that;
}

String *mstring_as_string(MutableString *self)
{
    String *that = string_alloc(self->base.length);
    if(NULL == self)
    {
        return NULL;
    }
    return string_init(that, (const char *)self->base.value, self->base.length);
}

String *mstring_as_string_no_copy(MutableString *self)
{
    return &self->base;
}

const char *mstring_as_c_str(MutableString *self)
{
    return (const char *)self->base.value;
}

static inline size_t calculate_new_capacity(size_t capacity)
{
    return (capacity * 3) / 2 + 1;
}

static inline bool ensure_capacity(MutableString **self, size_t length)
{
    if(!mstring_has_capacity(*self, length))
    {
        size_t min_capacity = (*self)->base.length + length;
        size_t new_capacity = calculate_new_capacity(min_capacity);
        return mstring_realloc(self, new_capacity);
    }
    return true;
}

static inline void append(MutableString *self, const void *data, size_t length)
{
    memcpy(self->base.value + self->base.length, data, length);
    self->base.length += length;
    self->base.value[self->base.length] = '\0';
}

bool mstring_append_char(MutableString **self, const uint8_t value)
{
    if(!ensure_capacity(self, 1))
    {
        return false;
    }
    append(*self, &value, 1);
    return true;
}

bool mstring_append_c_str(MutableString **self, const char *value)
{
    size_t length = strlen(value);
    if(!ensure_capacity(self, length))
    {
        return false;
    }
    append(*self, value, length);
    return true;
}

bool mstring_append_string(MutableString **self, const String *string)
{
    if(!ensure_capacity(self, string->length))
    {
        return false;
    }
    append(*self, string->value, string->length);
    return true;
}

bool mstring_append_mstring(MutableString **self, const MutableString *string)
{
    if(!ensure_capacity(self, string->base.length))
    {
        return false;
    }
    append(*self, string->base.value, string->base.length);
    return true;
}

bool mstring_append_stream(MutableString **self, const uint8_t *value, size_t length)
{
    if(!ensure_capacity(self, length))
    {
        return false;
    }
    append(*self, value, length);
    return true;
}

void mstring_set(MutableString *self, size_t position, uint8_t value)
{
    if(position > self->base.length)
    {
        return;
    }
    self->base.value[position] = value;
}

void mstring_set_range(MutableString *self, size_t position, size_t length, const uint8_t *value)
{
    if(position > self->base.length || position + length > self->base.length)
    {
        return;
    }
    memcpy(self->base.value, value, length);
}
