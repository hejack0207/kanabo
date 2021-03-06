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


#include <ctype.h>
#include <stdio.h>

#include "emit/shell.h"
#include "log.h"


#define MAYBE_EMIT(STR) if(context->wrap_collections)   \
    {                                                   \
        EMIT((STR));                                    \
    }

bool emit_node(Node *each, void *argument)
{
    emit_context *context = (emit_context *)argument;

    log_debug("shell", "emitting node...");
    bool result = true;
    switch(node_kind(each))
    {
        case DOCUMENT:
            log_trace("shell", "emitting document");
            result = emit_node(document_root(document(each)), NULL);
            break;
        case SCALAR:
            result = emit_scalar(scalar(each));
            EMIT("\n");
            break;
        case SEQUENCE:
            log_trace("shell", "emitting seqence");
            MAYBE_EMIT("(");
            result = sequence_iterate(sequence(each), emit_sequence_item, NULL);
            MAYBE_EMIT(")");
            EMIT("\n");
            break;
        case MAPPING:
            log_trace("shell", "emitting mapping");
            MAYBE_EMIT("(");
            result = mapping_iterate(mapping(each), context->emit_mapping_item, NULL);
            MAYBE_EMIT(")");
            EMIT("\n");
            break;
        case ALIAS:
            log_trace("shell", "resolving alias");
            result = emit_node(alias_target(alias(each)), NULL);
            break;
    }

    return result;
}

static bool scalar_contains_space(const Scalar *each)
{
    uint8_t *value = scalar_value(each);
    for(size_t i = 0; i < node_size(each); i++)
    {
        if(isspace(*(value + i)))
        {
            return true;
        }
    }

    return false;
}

bool emit_scalar(const Scalar *each)
{
    if(SCALAR_STRING == scalar_kind(each) && scalar_contains_space(each))
    {
        log_trace("shell", "emitting quoted scalar");
        return emit_quoted_scalar(each);
    }
    else
    {
        log_trace("shell", "emitting raw scalar");
        return emit_raw_scalar(each);
    }
}

bool emit_quoted_scalar(const Scalar *each)
{
    EMIT("'");
    if(!emit_raw_scalar(each))
    {
        log_error("shell", "uh oh! couldn't emit quoted scalar");
        return false;
    }
    EMIT("'");

    return true;
}

bool emit_raw_scalar(const Scalar *each)
{
    return 1 == fwrite(scalar_value(each), node_size(each), 1, stdout);
}

bool emit_sequence_item(Node *each, void *context __attribute__((unused)))
{
    if(is_scalar(each))
    {
        log_trace("shell", "emitting sequence item");
        if(!emit_scalar(scalar(each)))
        {
            return false;
        }
        EMIT(" ");
    }
    else
    {
        log_trace("shell", "skipping sequence item");
    }

    return true;
}
