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

#include <stdio.h>
#include <string.h>

#include "jsonpath.h"
#include "conditions.h"

static const char * const MESSAGES[] = 
{
    "Success.",
    "Expression argument was NULL.",
    "Expression length was 0.",
    "Unable to allocate memory.",
    "Not a JSONPath expression.",
    "Premature end of input after position %d.",
    "At position %d: unexpected character '%c', was expecting '%c' instead.",
    "At position %d: empty predicate.",
    "At position %d: missing closing predicate delimiter `]' before end of step.",
    "At position %d: unsupported predicate found.",
    "At position %d: extra characters after valid predicate definition.",
    "At position %d: expected a name character, but found '%c' instead.",
    "At position %d: expected a node type test.",
    "At position %d: expected an integer.",
    "At position %d: invalid number.",
    "At position %d: slice step value must be non-zero."
};

char *parser_status_message(const parser_context * restrict context)
{
    PRECOND_NONNULL_ELSE_NULL(context);
    char *message = NULL;
    
    switch(context->result.code)
    {
        case ERR_PREMATURE_END_OF_INPUT:
            asprintf(&message, MESSAGES[context->result.code], context->cursor);
            break;
        case ERR_EXPECTED_NODE_TYPE_TEST:
        case ERR_EMPTY_PREDICATE:
        case ERR_UNBALANCED_PRED_DELIM:
        case ERR_EXTRA_JUNK_AFTER_PREDICATE:
        case ERR_UNSUPPORTED_PRED_TYPE:
        case ERR_EXPECTED_INTEGER:
        case ERR_INVALID_NUMBER:
            asprintf(&message, MESSAGES[context->result.code], context->cursor + 1);
            break;
        case ERR_UNEXPECTED_VALUE:
            asprintf(&message, MESSAGES[context->result.code], context->cursor + 1, context->result.actual_char, context->result.expected_char);
            break;
        case ERR_EXPECTED_NAME_CHAR:
            asprintf(&message, MESSAGES[context->result.code], context->cursor + 1, context->result.actual_char);
            break;
        default:
            message = strdup(MESSAGES[context->result.code]);
            break;
    }

    return message;
}
