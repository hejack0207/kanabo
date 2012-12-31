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

#include "jsonpath.h"
#include "test.h"

static void assert_parser_result(parser_result *result, jsonpath *path, enum path_kind expected_kind, size_t expected_length);

static void assert_root_step(jsonpath *path);
static void assert_single_name_step(jsonpath *path, size_t index, char *name);
static void assert_recursive_name_step(jsonpath *path, size_t index, char *name);
static void assert_name_step(jsonpath *path, size_t index, char *name, enum step_kind expected_step_kind);
static void assert_single_type_step(jsonpath *path, size_t index, enum type_test_kind expected_type_kind);
static void assert_recursive_type_step(jsonpath *path, size_t index, enum type_test_kind expected_type_kind);
static void assert_type_step(jsonpath *path, size_t index, enum type_test_kind expected_type_kind, enum step_kind expected_step_kind);
static void assert_step(jsonpath *path, size_t index, enum step_kind expected_step_kind, enum test_kind expected_test_kind);
static void assert_name(step * step, uint8_t *value, size_t length);
static void assert_no_predicates(jsonpath *path, size_t index);
static void assert_predicate(jsonpath *path, size_t path_index, size_t predicate_index, enum predicate_kind expected_predicate_kind);

START_TEST (null_expression)
{
    jsonpath path;
    parser_result *result = parse_jsonpath(NULL, 50, &path);
    
    ck_assert_not_null(result);
    ck_assert_int_eq(ERR_NULL_EXPRESSION, result->code);
    ck_assert_not_null(result->message);

    fprintf(stdout, "received expected failure message: '%s'\n", result->message);

    ck_assert_int_eq(0, result->position);

    free_parser_result(result);
}
END_TEST

START_TEST (zero_length)
{
    jsonpath path;
    parser_result *result = parse_jsonpath((uint8_t *)"", 0, &path);
    
    ck_assert_not_null(result);
    ck_assert_int_eq(ERR_ZERO_LENGTH, result->code);
    ck_assert_not_null(result->message);

    fprintf(stdout, "received expected failure message: '%s'\n", result->message);

    ck_assert_int_eq(0, result->position);

    free_parser_result(result);
}
END_TEST

START_TEST (null_path)
{
    parser_result *result = parse_jsonpath((uint8_t *)"", 5, NULL);
    
    ck_assert_not_null(result);
    ck_assert_int_eq(ERR_NULL_OUTPUT_PATH, result->code);
    ck_assert_not_null(result->message);

    fprintf(stdout, "received expected failure message: '%s'\n", result->message);

    ck_assert_int_eq(0, result->position);

    free_parser_result(result);
}
END_TEST

START_TEST (missing_step_test)
{
    jsonpath path;
    parser_result *result = parse_jsonpath((uint8_t *)"$.", 2, &path);
    
    ck_assert_not_null(result);
    ck_assert_int_eq(ERR_PREMATURE_END_OF_INPUT, result->code);
    ck_assert_int_eq(3, result->position);
    ck_assert_not_null(result->message);

    fprintf(stdout, "received expected failure message: '%s'\n", result->message);

    free_parser_result(result);
    free_jsonpath(&path);
}
END_TEST

START_TEST (missing_recursive_step_test)
{
    jsonpath path;
    parser_result *result = parse_jsonpath((uint8_t *)"$..", 3, &path);
    
    ck_assert_not_null(result);
    ck_assert_int_eq(ERR_PREMATURE_END_OF_INPUT, result->code);
    ck_assert_int_eq(4, result->position);
    ck_assert_not_null(result->message);

    fprintf(stdout, "received expected failure message: '%s'\n", result->message);

    free_parser_result(result);
    free_jsonpath(&path);
}
END_TEST

START_TEST (missing_dot)
{
    jsonpath path;
    parser_result *result = parse_jsonpath((uint8_t *)"$x", 2, &path);
    
    ck_assert_not_null(result);
    ck_assert_int_eq(ERR_UNEXPECTED_VALUE, result->code);
    ck_assert_int_eq(2, result->position);
    ck_assert_not_null(result->message);

    fprintf(stdout, "received expected failure message: '%s'\n", result->message);

    free_parser_result(result);
    free_jsonpath(&path);
}
END_TEST

START_TEST (relative_path_begins_with_dot)
{
    jsonpath path;
    parser_result *result = parse_jsonpath((uint8_t *)".x", 2, &path);
    
    ck_assert_not_null(result);
    ck_assert_int_eq(ERR_EXPECTED_NAME_CHAR, result->code);
    ck_assert_int_eq(1, result->position);
    ck_assert_not_null(result->message);

    fprintf(stdout, "received expected failure message: '%s'\n", result->message);

    free_parser_result(result);
    free_jsonpath(&path);
}
END_TEST

START_TEST (quoted_empty_step)
{
    jsonpath path;
    char *expression = "$.foo.''.bar";
    parser_result *result = parse_jsonpath((uint8_t *)expression, strlen(expression), &path);
    
    ck_assert_not_null(result);
    ck_assert_int_eq(ERR_EXPECTED_NAME_CHAR, result->code);
    ck_assert_int_eq(8, result->position);
    ck_assert_not_null(result->message);

    fprintf(stdout, "received expected failure message: '%s'\n", result->message);

    free_parser_result(result);
    free_jsonpath(&path);
}
END_TEST

START_TEST (empty_predicate)
{
    jsonpath path;
    char *expression = "$.foo[].bar";
    parser_result *result = parse_jsonpath((uint8_t *)expression, strlen(expression), &path);
    
    ck_assert_not_null(result);
    ck_assert_int_eq(ERR_EMPTY_PREDICATE, result->code);
    ck_assert_int_eq(7, result->position);
    ck_assert_not_null(result->message);

    fprintf(stdout, "received expected failure message: '%s'\n", result->message);

    free_parser_result(result);
    free_jsonpath(&path);
}
END_TEST

START_TEST (extra_junk_in_predicate)
{
    jsonpath path;
    char *expression = "$.foo[ * quux].bar";
    parser_result *result = parse_jsonpath((uint8_t *)expression, strlen(expression), &path);
    
    ck_assert_not_null(result);
    ck_assert_int_eq(ERR_EXTRA_JUNK_AFTER_PREDICATE, result->code);
    ck_assert_int_eq(10, result->position);
    ck_assert_not_null(result->message);

    fprintf(stdout, "received expected failure message: '%s'\n", result->message);

    free_parser_result(result);
    free_jsonpath(&path);
}
END_TEST

START_TEST (whitespace_predicate)
{
    jsonpath path;
    char *expression = "$.foo[ \t ].bar";
    parser_result *result = parse_jsonpath((uint8_t *)expression, strlen(expression), &path);
    
    ck_assert_not_null(result);
    ck_assert_int_eq(ERR_EMPTY_PREDICATE, result->code);
    ck_assert_int_eq(10, result->position);
    ck_assert_not_null(result->message);

    fprintf(stdout, "received expected failure message: '%s'\n", result->message);

    free_parser_result(result);
    free_jsonpath(&path);
}
END_TEST

START_TEST (bogus_type_test_name)
{
    jsonpath path;
    char *expression = "$.foo.monkey()";
    parser_result *result = parse_jsonpath((uint8_t *)expression, strlen(expression), &path);
    
    ck_assert_not_null(result);
    ck_assert_int_eq(ERR_EXPECTED_NODE_TYPE_TEST, result->code);
    ck_assert_int_eq(7, result->position);
    ck_assert_not_null(result->message);

    fprintf(stdout, "received expected failure message: '%s'\n", result->message);

    free_parser_result(result);
    free_jsonpath(&path);
}
END_TEST

START_TEST (bogus_type_test_name_oblong)
{
    jsonpath path;
    char *expression = "$.foo.oblong()";
    parser_result *result = parse_jsonpath((uint8_t *)expression, strlen(expression), &path);
    
    ck_assert_not_null(result);
    ck_assert_int_eq(ERR_EXPECTED_NODE_TYPE_TEST, result->code);
    ck_assert_int_eq(7, result->position);
    ck_assert_not_null(result->message);

    fprintf(stdout, "received expected failure message: '%s'\n", result->message);

    free_parser_result(result);
    free_jsonpath(&path);
}
END_TEST

START_TEST (bogus_type_test_name_alloy)
{
    jsonpath path;
    char *expression = "$.foo.alloy()";
    parser_result *result = parse_jsonpath((uint8_t *)expression, strlen(expression), &path);
    
    ck_assert_not_null(result);
    ck_assert_int_eq(ERR_EXPECTED_NODE_TYPE_TEST, result->code);
    ck_assert_int_eq(7, result->position);
    ck_assert_not_null(result->message);

    fprintf(stdout, "received expected failure message: '%s'\n", result->message);

    free_parser_result(result);
    free_jsonpath(&path);
}
END_TEST

START_TEST (bogus_type_test_name_strong)
{
    jsonpath path;
    char *expression = "$.foo.strong()";
    parser_result *result = parse_jsonpath((uint8_t *)expression, strlen(expression), &path);
    
    ck_assert_not_null(result);
    ck_assert_int_eq(ERR_EXPECTED_NODE_TYPE_TEST, result->code);
    ck_assert_int_eq(7, result->position);
    ck_assert_not_null(result->message);

    fprintf(stdout, "received expected failure message: '%s'\n", result->message);

    free_parser_result(result);
    free_jsonpath(&path);
}
END_TEST

START_TEST (bogus_type_test_name_numred)
{
    jsonpath path;
    char *expression = "$.foo.numred()";
    parser_result *result = parse_jsonpath((uint8_t *)expression, strlen(expression), &path);
    
    ck_assert_not_null(result);
    ck_assert_int_eq(ERR_EXPECTED_NODE_TYPE_TEST, result->code);
    ck_assert_int_eq(7, result->position);
    ck_assert_not_null(result->message);

    fprintf(stdout, "received expected failure message: '%s'\n", result->message);

    free_parser_result(result);
    free_jsonpath(&path);
}
END_TEST

START_TEST (bogus_type_test_name_booloud)
{
    jsonpath path;
    char *expression = "$.foo.booloud()";
    parser_result *result = parse_jsonpath((uint8_t *)expression, strlen(expression), &path);
    
    ck_assert_not_null(result);
    ck_assert_int_eq(ERR_EXPECTED_NODE_TYPE_TEST, result->code);
    ck_assert_int_eq(7, result->position);
    ck_assert_not_null(result->message);

    fprintf(stdout, "received expected failure message: '%s'\n", result->message);

    free_parser_result(result);
    free_jsonpath(&path);
}
END_TEST

START_TEST (bogus_type_test_name_narl)
{
    jsonpath path;
    char *expression = "$.foo.narl()";
    parser_result *result = parse_jsonpath((uint8_t *)expression, strlen(expression), &path);
    
    ck_assert_not_null(result);
    ck_assert_int_eq(ERR_EXPECTED_NODE_TYPE_TEST, result->code);
    ck_assert_int_eq(7, result->position);
    ck_assert_not_null(result->message);

    fprintf(stdout, "received expected failure message: '%s'\n", result->message);

    free_parser_result(result);
    free_jsonpath(&path);
}
END_TEST

START_TEST (empty_type_test_name)
{
    jsonpath path;
    char *expression = "$.foo.()";
    parser_result *result = parse_jsonpath((uint8_t *)expression, strlen(expression), &path);
    
    ck_assert_not_null(result);
    ck_assert_int_eq(ERR_EXPECTED_NODE_TYPE_TEST, result->code);
    ck_assert_int_eq(7, result->position);
    ck_assert_not_null(result->message);

    fprintf(stdout, "received expected failure message: '%s'\n", result->message);

    free_parser_result(result);
    free_jsonpath(&path);
}
END_TEST

// xxx - type test with whitespace inside parens

static void assert_parser_result(parser_result *result, jsonpath *path, enum path_kind expected_kind, size_t expected_length)
{
    ck_assert_not_null(result);
    ck_assert_int_eq(SUCCESS, result->code);
    ck_assert_not_null(result->message);
    ck_assert_int_ne(0, result->position);
    ck_assert_int_eq(expected_kind, path->kind);
    ck_assert_not_null(path->steps);
    ck_assert_int_eq(expected_length, path->length);
}

static void assert_root_step(jsonpath *path)
{
    assert_step(path, 0, ROOT, NAME_TEST);
    assert_no_predicates(path, 0);
}

static void assert_single_name_step(jsonpath *path, size_t index, char *name)
{
    assert_name_step(path, index, name, SINGLE);
}

static void assert_recursive_name_step(jsonpath *path, size_t index, char *name)
{
    assert_name_step(path, index, name, RECURSIVE);
}

static void assert_name_step(jsonpath *path, size_t index, char *name, enum step_kind expected_step_kind)
{
    assert_step(path, index, expected_step_kind, NAME_TEST);
    assert_name(path->steps[index], (uint8_t *)name, strlen(name));
}

static void assert_single_type_step(jsonpath *path, size_t index, enum type_test_kind expected_type_kind)
{
    assert_type_step(path, index, expected_type_kind, SINGLE);
}

static void assert_recursive_type_step(jsonpath *path, size_t index, enum type_test_kind expected_type_kind)
{
    assert_type_step(path, index, expected_type_kind, RECURSIVE);
}

static void assert_type_step(jsonpath *path, size_t index, enum type_test_kind expected_type_kind, enum step_kind expected_step_kind)
{
    assert_step(path, index, expected_step_kind, TYPE_TEST);
    ck_assert_int_eq(expected_type_kind, path->steps[index]->test.type);
}

static void assert_step(jsonpath *path, size_t index, enum step_kind expected_step_kind, enum test_kind expected_test_kind)
{
    ck_assert_int_eq(expected_step_kind, path->steps[index]->kind);
    ck_assert_int_eq(expected_test_kind, path->steps[index]->test.kind);
}

static void assert_name(step * step, uint8_t *name, size_t length)
{
    ck_assert_int_eq(length, step->test.name.length);
    ck_assert_buf_eq(name, length, step->test.name.value, step->test.name.length);
}

static void assert_no_predicates(jsonpath *path, size_t index)
{
    ck_assert_int_eq(0, path->steps[index]->predicate_count);
    ck_assert_null(path->steps[index]->predicates);
}

static void assert_predicate(jsonpath *path, size_t path_index, size_t predicate_index, enum predicate_kind expected_predicate_kind)
{
    ck_assert_int_ne(0, path->steps[path_index]->predicate_count);
    ck_assert_not_null(path->steps[path_index]->predicates);
    ck_assert_not_null(path->steps[path_index]->predicates[predicate_index]);
    ck_assert_int_eq(expected_predicate_kind, path->steps[path_index]->predicates[predicate_index]->kind);
}

START_TEST (dollar_only)
{
    jsonpath path;
    parser_result *result = parse_jsonpath((uint8_t *)"$", 1, &path);
    
    assert_parser_result(result, &path, ABSOLUTE_PATH, 1);
    assert_root_step(&path);

    free_parser_result(result);
    free_jsonpath(&path);
}
END_TEST

START_TEST (absolute_single_step)
{
    jsonpath path;
    parser_result *result = parse_jsonpath((uint8_t *)"$.foo", 5, &path);
    
    assert_parser_result(result, &path, ABSOLUTE_PATH, 2);
    assert_root_step(&path);
    assert_single_name_step(&path, 1, "foo");
    assert_no_predicates(&path, 0);

    free_parser_result(result);
    free_jsonpath(&path);    
}
END_TEST

START_TEST (absolute_recursive_step)
{
    jsonpath path;
    parser_result *result = parse_jsonpath((uint8_t *)"$..foo", 6, &path);
    
    assert_parser_result(result, &path, ABSOLUTE_PATH, 2);
    assert_root_step(&path);
    assert_recursive_name_step(&path, 1, "foo");
    assert_no_predicates(&path, 0);

    free_parser_result(result);
    free_jsonpath(&path);    
}
END_TEST

START_TEST (absolute_multi_step)
{
    jsonpath path;
    char *expression = "$.foo.baz..yobble.thingum";
    parser_result *result = parse_jsonpath((uint8_t *)expression, strlen(expression), &path);
    
    assert_parser_result(result, &path, ABSOLUTE_PATH, 5);
    assert_root_step(&path);
    assert_single_name_step(&path, 1, "foo");
    assert_single_name_step(&path, 2, "baz");
    assert_recursive_name_step(&path, 3, "yobble");
    assert_single_name_step(&path, 4, "thingum");
    assert_no_predicates(&path, 1);
    assert_no_predicates(&path, 2);
    assert_no_predicates(&path, 3);
    assert_no_predicates(&path, 4);

    free_parser_result(result);
    free_jsonpath(&path);    
}
END_TEST

START_TEST (relative_multi_step)
{
    jsonpath path;
    char *expression = "foo.bar..baz";
    parser_result *result = parse_jsonpath((uint8_t *)expression, strlen(expression), &path);
    
    assert_parser_result(result, &path, RELATIVE_PATH, 3);
    assert_single_name_step(&path, 0, "foo");
    assert_single_name_step(&path, 1, "bar");
    assert_recursive_name_step(&path, 2, "baz");
    assert_no_predicates(&path, 0);
    assert_no_predicates(&path, 1);
    assert_no_predicates(&path, 2);

    free_parser_result(result);
    free_jsonpath(&path);    
}
END_TEST

START_TEST (quoted_multi_step)
{
    jsonpath path;
    char *expression = "$.foo.'happy fun ball'.bar";
    parser_result *result = parse_jsonpath((uint8_t *)expression, strlen(expression), &path);
    
    assert_parser_result(result, &path, ABSOLUTE_PATH, 4);
    assert_root_step(&path);
    assert_single_name_step(&path, 1, "foo");
    assert_single_name_step(&path, 2, "happy fun ball");
    assert_single_name_step(&path, 3, "bar");
    assert_no_predicates(&path, 1);
    assert_no_predicates(&path, 2);
    assert_no_predicates(&path, 3);

    free_parser_result(result);
    free_jsonpath(&path);    
}
END_TEST

START_TEST (whitespace)
{
    jsonpath path;
    char *expression = "  $ \r\n. foo \n.\n. \t'happy fun ball' . \t string()";
    parser_result *result = parse_jsonpath((uint8_t *)expression, strlen(expression), &path);
    
    assert_parser_result(result, &path, ABSOLUTE_PATH, 4);
    assert_root_step(&path);
    assert_single_name_step(&path, 1, "foo");
    assert_recursive_name_step(&path, 2, "happy fun ball");
    assert_single_type_step(&path, 3, STRING_TEST);
    assert_no_predicates(&path, 1);
    assert_no_predicates(&path, 2);
    assert_no_predicates(&path, 3);

    free_parser_result(result);
    free_jsonpath(&path);    
}
END_TEST

START_TEST (type_test_missing_closing_paren)
{
    jsonpath path;
    char *expression = "$.foo.null(";
    parser_result *result = parse_jsonpath((uint8_t *)expression, strlen(expression), &path);
    
    assert_parser_result(result, &path, ABSOLUTE_PATH, 3);
    assert_root_step(&path);
    assert_single_name_step(&path, 1, "foo");
    assert_single_name_step(&path, 2, "null(");
    assert_no_predicates(&path, 1);
    assert_no_predicates(&path, 2);

    free_parser_result(result);
    free_jsonpath(&path);
}
END_TEST

START_TEST (type_test_with_trailing_junk)
{
    jsonpath path;
    char *expression = "$.foo.array()[0]";
    parser_result *result = parse_jsonpath((uint8_t *)expression, strlen(expression), &path);
    
    assert_parser_result(result, &path, ABSOLUTE_PATH, 3);
    assert_root_step(&path);
    assert_single_name_step(&path, 1, "foo");
    assert_single_type_step(&path, 2, ARRAY_TEST);
    assert_no_predicates(&path, 1);
    assert_no_predicates(&path, 2);

    free_parser_result(result);
    free_jsonpath(&path);
}
END_TEST

START_TEST (recursive_type_test)
{
    jsonpath path;
    char *expression = "$.foo..string()";
    parser_result *result = parse_jsonpath((uint8_t *)expression, strlen(expression), &path);
    
    assert_parser_result(result, &path, ABSOLUTE_PATH, 3);
    assert_root_step(&path);
    assert_single_name_step(&path, 1, "foo");
    assert_recursive_type_step(&path, 2, STRING_TEST);
    assert_no_predicates(&path, 1);
    assert_no_predicates(&path, 2);

    free_parser_result(result);
    free_jsonpath(&path);
}
END_TEST

START_TEST (object_type_test)
{
    jsonpath path;
    char *expression = "$.foo.object()";
    parser_result *result = parse_jsonpath((uint8_t *)expression, strlen(expression), &path);
    
    assert_parser_result(result, &path, ABSOLUTE_PATH, 3);
    assert_root_step(&path);
    assert_single_name_step(&path, 1, "foo");
    assert_single_type_step(&path, 2, OBJECT_TEST);
    assert_no_predicates(&path, 1);
    assert_no_predicates(&path, 2);

    free_parser_result(result);
    free_jsonpath(&path);
}
END_TEST

START_TEST (array_type_test)
{
    jsonpath path;
    char *expression = "$.foo.array()";
    parser_result *result = parse_jsonpath((uint8_t *)expression, strlen(expression), &path);
    
    assert_parser_result(result, &path, ABSOLUTE_PATH, 3);
    assert_root_step(&path);
    assert_single_name_step(&path, 1, "foo");
    assert_single_type_step(&path, 2, ARRAY_TEST);
    assert_no_predicates(&path, 1);
    assert_no_predicates(&path, 2);

    free_parser_result(result);
    free_jsonpath(&path);
}
END_TEST

START_TEST (string_type_test)
{
    jsonpath path;
    char *expression = "$.foo.string()";
    parser_result *result = parse_jsonpath((uint8_t *)expression, strlen(expression), &path);
    
    assert_parser_result(result, &path, ABSOLUTE_PATH, 3);
    assert_root_step(&path);
    assert_single_name_step(&path, 1, "foo");
    assert_single_type_step(&path, 2, STRING_TEST);
    assert_no_predicates(&path, 1);
    assert_no_predicates(&path, 2);

    free_parser_result(result);
    free_jsonpath(&path);
}
END_TEST

START_TEST (number_type_test)
{
    jsonpath path;
    char *expression = "$.foo.number()";
    parser_result *result = parse_jsonpath((uint8_t *)expression, strlen(expression), &path);
    
    assert_parser_result(result, &path, ABSOLUTE_PATH, 3);
    assert_root_step(&path);
    assert_single_name_step(&path, 1, "foo");
    assert_single_type_step(&path, 2, NUMBER_TEST);
    assert_no_predicates(&path, 1);
    assert_no_predicates(&path, 2);

    free_parser_result(result);
    free_jsonpath(&path);
}
END_TEST

START_TEST (boolean_type_test)
{
    jsonpath path;
    char *expression = "$.foo.boolean()";
    parser_result *result = parse_jsonpath((uint8_t *)expression, strlen(expression), &path);
    
    assert_parser_result(result, &path, ABSOLUTE_PATH, 3);
    assert_root_step(&path);
    assert_single_name_step(&path, 1, "foo");
    assert_single_type_step(&path, 2, BOOLEAN_TEST);
    assert_no_predicates(&path, 1);
    assert_no_predicates(&path, 2);

    free_parser_result(result);
    free_jsonpath(&path);
}
END_TEST

START_TEST (null_type_test)
{
    jsonpath path;
    char *expression = "$.foo.null()";
    parser_result *result = parse_jsonpath((uint8_t *)expression, strlen(expression), &path);
    
    assert_parser_result(result, &path, ABSOLUTE_PATH, 3);
    assert_root_step(&path);
    assert_single_name_step(&path, 1, "foo");
    assert_single_type_step(&path, 2, NULL_TEST);
    assert_no_predicates(&path, 1);
    assert_no_predicates(&path, 2);

    free_parser_result(result);
    free_jsonpath(&path);
}
END_TEST

START_TEST (wildcard_predicate)
{
    jsonpath path;
    char *expression = "$.store.book[*].author";
    parser_result *result = parse_jsonpath((uint8_t *)expression, strlen(expression), &path);
    
    assert_parser_result(result, &path, ABSOLUTE_PATH, 4);
    assert_root_step(&path);
    assert_single_name_step(&path, 1, "store");
    assert_no_predicates(&path, 1);
    assert_single_name_step(&path, 2, "book");
    assert_predicate(&path, 2, 0, WILDCARD);
    assert_single_name_step(&path, 3, "author");
    assert_no_predicates(&path, 3);

    free_parser_result(result);
    free_jsonpath(&path);
}
END_TEST

START_TEST (wildcard_predicate_with_whitespace)
{
    jsonpath path;
    char *expression = "$.foo  [\t*\n]  .bar";
    parser_result *result = parse_jsonpath((uint8_t *)expression, strlen(expression), &path);
    
    assert_parser_result(result, &path, ABSOLUTE_PATH, 3);
    assert_root_step(&path);
    assert_single_name_step(&path, 1, "foo");
    assert_predicate(&path, 1, 0, WILDCARD);
    assert_single_name_step(&path, 2, "bar");
    assert_no_predicates(&path, 2);

    free_parser_result(result);
    free_jsonpath(&path);
}
END_TEST

Suite *jsonpath_suite(void)
{
    TCase *bad_input = tcase_create("bad input");
    tcase_add_test(bad_input, null_expression);
    tcase_add_test(bad_input, zero_length);
    tcase_add_test(bad_input, null_path);
    tcase_add_test(bad_input, missing_step_test);
    tcase_add_test(bad_input, missing_recursive_step_test);
    tcase_add_test(bad_input, missing_dot);
    tcase_add_test(bad_input, relative_path_begins_with_dot);
    tcase_add_test(bad_input, quoted_empty_step);
    tcase_add_test(bad_input, bogus_type_test_name);
    tcase_add_test(bad_input, bogus_type_test_name_oblong);
    tcase_add_test(bad_input, bogus_type_test_name_alloy);
    tcase_add_test(bad_input, bogus_type_test_name_strong);
    tcase_add_test(bad_input, bogus_type_test_name_numred);
    tcase_add_test(bad_input, bogus_type_test_name_booloud);
    tcase_add_test(bad_input, bogus_type_test_name_narl);
    tcase_add_test(bad_input, empty_type_test_name);
    tcase_add_test(bad_input, empty_predicate);
    tcase_add_test(bad_input, whitespace_predicate);
    tcase_add_test(bad_input, extra_junk_in_predicate);

    TCase *basic = tcase_create("basic");
    tcase_add_test(basic, dollar_only);
    tcase_add_test(basic, absolute_single_step);
    tcase_add_test(basic, absolute_recursive_step);
    tcase_add_test(basic, absolute_multi_step);
    tcase_add_test(basic, quoted_multi_step);
    tcase_add_test(basic, relative_multi_step);
    tcase_add_test(basic, whitespace);

    TCase *node_type = tcase_create("node type test");
    tcase_add_test(node_type, type_test_missing_closing_paren);
    tcase_add_test(node_type, type_test_with_trailing_junk);
    tcase_add_test(node_type, recursive_type_test);
    tcase_add_test(node_type, object_type_test);
    tcase_add_test(node_type, array_type_test);
    tcase_add_test(node_type, string_type_test);
    tcase_add_test(node_type, number_type_test);
    tcase_add_test(node_type, boolean_type_test);
    tcase_add_test(node_type, null_type_test);

    TCase *predicate = tcase_create("predicate");
    tcase_add_test(predicate, wildcard_predicate);
    tcase_add_test(predicate, wildcard_predicate_with_whitespace);
    
    Suite *jsonpath = suite_create("JSONPath");
    suite_add_tcase(jsonpath, bad_input);
    suite_add_tcase(jsonpath, basic);
    suite_add_tcase(jsonpath, node_type);
    suite_add_tcase(jsonpath, predicate);

    return jsonpath;
}
