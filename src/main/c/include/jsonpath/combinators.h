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

#pragma once

#include <stddef.h>

#include "jsonpath/parsers.h"

enum combinator_kind
{
    CHOICE,
    SEQUENCE,
    OPTION,
    REPETITION,
    TERMINAL,
    RULE,
};

struct parser_s
{
    enum combinator_kind kind;
    parser_function      parser;
};

typedef struct parser_s Parser;

/* Destructor */
void parser_free(Parser *value);

/* Non-terminal parsers */
Parser *rule_combinator(const char *name, Parser *expression);
#define rule(COMBINATOR) rule_combinator(__func__, (COMBINATOR))

Parser *choice_combinator(Parser *one, Parser *two, ...);
#define choice(...) choice_combinator(__VA_ARGS__, NULL)

Parser *sequence_combinator(Parser *one, Parser *two, ...);
#define sequence(...) sequence_combinator(__VA_ARGS__, NULL)

Parser *option(Parser *optional);
Parser *repetition(Parser *repeated);

/* Terminal parsers */
Parser *literal(char *value);

Parser *number(void);
Parser *integer(void);
Parser *signed_integer(void);
Parser *non_zero_signed_integer(void);

Parser *quoted_string(void);
Parser *string(void);