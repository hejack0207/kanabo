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

#include "jsonpath/model.h"
#include "conditions.h"

static bool slice_predicate_has(const Predicate *value, enum slice_specifiers specifier);


int_fast32_t slice_predicate_to(const Predicate *value)
{
    PRECOND_NONNULL_ELSE_ZERO(value);
    PRECOND_ELSE_ZERO(SLICE == value->kind);
    return value->slice.to;
}

int_fast32_t slice_predicate_from(const Predicate *value)
{
    PRECOND_NONNULL_ELSE_ZERO(value);
    PRECOND_ELSE_ZERO(SLICE == value->kind);
    return value->slice.from;
}

int_fast32_t slice_predicate_step(const Predicate *value)
{
    PRECOND_NONNULL_ELSE_ZERO(value);
    PRECOND_ELSE_ZERO(SLICE == value->kind);
    return value->slice.step;
}

bool slice_predicate_has_to(const Predicate *value)
{
    return slice_predicate_has(value, SLICE_TO);
}

bool slice_predicate_has_from(const Predicate *value)
{
    return slice_predicate_has(value, SLICE_FROM);
}

bool slice_predicate_has_step(const Predicate *value)
{
    return slice_predicate_has(value, SLICE_STEP);
}

static bool slice_predicate_has(const Predicate *value, enum slice_specifiers specifier)
{
    PRECOND_NONNULL_ELSE_FALSE(value);
    PRECOND_ELSE_FALSE(SLICE == value->kind);
    return value->slice.specified & specifier;
}
