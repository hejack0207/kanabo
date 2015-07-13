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


#include "jsonpath/combinators/base.h"


static const char * const PARSER_NAMES[] =
{
    "rule"
    "choice",
    "sequence",
    "option",
    "repetition",
    "literal",
    "number",
    "integer",
    "signed_integer",
    "non_zero_signed_integer",
    "quoted_string",
    "string"
};


static void generic_parser_free(Parser *self __attribute__((unused)))
{
}

static void generic_parser_log(Parser *self)
{
    parser_debug("processing %s parser, value: %s", parser_name(self));
}

static const struct vtable_s PARSER_VTABLE =
{
    generic_parser_free,
    generic_parser_log
};


Parser *make_parser(enum parser_kind kind, parser_function function)
{
    Parser *parser = (Parser *)calloc(1, sizeof(Parser));
    return parser_init(parser, kind, function, &PARSER_VTABLE);
}

Parser *parser_init(Parser *self,
                    enum parser_kind kind,
                    parser_function function,
                    const struct vtable_s *vtable)
{
    if(NULL == self)
    {
        return NULL;
    }
    self->kind = kind;
    self->function = function;
    self->vtable = vtable;

    return self;
}

void parser_free(Parser *self)
{
    if(NULL == self)
    {
        return;
    }
    self->vtable->free(self);
    free(self);
}

void parser_destructor(void *each)
{
    parser_free((Parser *)each);
}

enum parser_kind parser_kind(Parser *self)
{
    return self->kind;
}

const char *parser_name(Parser *self)
{
    return PARSER_NAMES[parser_kind(self)];
}
