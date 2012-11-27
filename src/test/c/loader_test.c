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
#include <check.h>

#include "loader.h"
#include "test.h"

static const unsigned char * const yaml = (unsigned char *)
    "one:\n"
    "  - foo1"
    "  - bar1\n"
    "\n"
    "two: foo2\n"
    "\n"
    "three: foo3\n"
    "\n"
    "four:\n"
    "  - foo4\n"
    "  - bar4\n"
    "\n"
    "five: foo5\n";

static const size_t yaml_size = 83;

static void assert_model_state(int result, document_model *model);

START_TEST (load_from_file)
{
    FILE *input = tmpfile();
    size_t written = fwrite(yaml, sizeof(char), yaml_size, input);
    // xxx - use fmemopen impl here instead of temp file
    ck_assert_int_eq(written, yaml_size);
    rewind(input);

    document_model model;
    int result = build_model_from_file(input, &model);

    assert_model_state(result, &model);
}
END_TEST

START_TEST (load_from_string)
{
    document_model model;
    int result = build_model_from_string(yaml, yaml_size, &model);

    assert_model_state(result, &model);
}
END_TEST

static void assert_model_state(int result, document_model *model)
{
    ck_assert_int_eq(0, result);
    ck_assert_int_eq(1, model->size);

    node *document = model->documents[0];
    
    ck_assert_int_eq(DOCUMENT, document->tag.kind);

    node *root = document->content.document.root;
    
    ck_assert_int_eq(MAPPING, root->tag.kind);
    ck_assert_int_eq(5, root->content.size);
}

Suite *loader_suite()
{
    TCase *loader = tcase_create("loader");
    tcase_add_test(loader, load_from_file);
    tcase_add_test(loader, load_from_string);

    Suite *model = suite_create("Model");
    suite_add_tcase(model, loader);
    
    return model;
}
