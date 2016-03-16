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


#include "vector.h"

#include "parsers/compound.h"


static MaybeSyntaxNode choice_delegate(Parser *parser, MaybeSyntaxNode node, Input *input)
{
    ensure_more_input(input);
    CompoundParser *self = (CompoundParser *)parser;

    set_mark(input);
    for(size_t i = 0; i < vector_length(self->children); i++)
    {
        reset_to_mark(input);
        Parser *each = vector_get(self->children, i);

        MaybeSyntaxNode result = bind(each, node, input);
        if(is_value(result))
        {
            syntax_node_add_child(node.value, result.value);
            return node;
        }
        else if(ERR_PREMATURE_END_OF_INPUT == result.code)
        {
            return result;
        }
    }

    return nothing_node(ERR_UNEXPECTED_VALUE);
}

Parser *choice_parser(Parser *one, Parser *two, ...)
{
    if(NULL == one || NULL == two)
    {
        if(NULL != one)
        {
            parser_free(one);
        }
        if(NULL != two)
        {
            parser_free(two);
        }
        return NULL;
    }
    va_list rest;
    va_start(rest, two);
    CompoundParser *self = make_compound_parser(CHOICE, one, two, rest);
    va_end(rest);
    if(NULL == self)
    {
        return NULL;
    }
    self->base.vtable.delegate = choice_delegate;
    asprintf(&self->base.repr, "choice %zd branches", vector_length(self->children));

    return (Parser *)self;
}