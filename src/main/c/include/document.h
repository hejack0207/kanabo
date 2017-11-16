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

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "hashtable.h"
#include "vector.h"

enum node_kind 
{
    DOCUMENT,
    SCALAR,
    SEQUENCE,
    MAPPING,
    ALIAS
};

enum scalar_kind
{
    SCALAR_STRING,
    SCALAR_INTEGER,
    SCALAR_REAL,
    SCALAR_TIMESTAMP,
    SCALAR_BOOLEAN,
    SCALAR_NULL
};

struct node
{
    struct node *parent;
    struct 
    {
        enum node_kind  kind;
        uint8_t        *name;
    } tag;

    uint8_t *anchor;
    struct
    {
        size_t size;
        union
        {
            struct
            {
                enum scalar_kind kind;
                uint8_t         *value;
            } scalar;
        
            Vector      *sequence;
            Hashtable   *mapping;
            struct node *target;
        };
    } content;
};

typedef struct node node;

struct model
{
    Vector *documents;
};

typedef struct model document_model;

/*
 * Constructors
 */

node *make_document_node(void);
node *make_sequence_node(void);
node *make_mapping_node(void);
node *make_scalar_node(const uint8_t *value, size_t length, enum scalar_kind kind);
node *make_alias_node(node *target);
document_model *make_model(void);

/*
 * Destructors
 */

void node_free(node *value);
void model_free(document_model *model);

/*
 * Model API
 */

node   *model_document(const document_model *model, size_t index);
node   *model_document_root(const document_model *model, size_t index);
size_t  model_document_count(const document_model *model);

bool model_add(document_model *model, node *document);

/*
 * Node API
 */

enum node_kind  node_kind(const node *value);
uint8_t        *node_name(const node *value);
size_t          node_size(const node *value);
node           *node_parent(const node *value);

bool node_equals(const node *one, const node *two);

void node_set_tag(node *target, const uint8_t *value, size_t length);
void node_set_anchor(node *target, const uint8_t *value, size_t length);

/*
 * Document API
 */

node *document_root(const node *document);
bool  document_set_root(node *document, node *root);

/*
 * Scalar API
 */

uint8_t *scalar_value(const node *scalar);
enum scalar_kind scalar_kind(const node *scalar);
bool scalar_boolean_is_true(const node *scalar);
bool scalar_boolean_is_false(const node *scalar);

/*
 * Sequence API
 */

node *sequence_get(const node *sequence, size_t index);
bool  sequence_add(node *sequence, node *item);

typedef bool (*sequence_iterator)(node *each, void *context);
bool sequence_iterate(const node *sequence, sequence_iterator iterator, void *context);

/*
 * Mapping API
 */

node *mapping_get(const node *mapping, uint8_t *key, size_t length);
bool  mapping_contains(const node *mapping, uint8_t *scalar, size_t length);
bool  mapping_put(node *mapping, uint8_t *key, size_t length, node *value);

typedef bool (*mapping_iterator)(node *key, node *value, void *context);
bool mapping_iterate(const node *mapping, mapping_iterator iterator, void *context);

/*
 * Alias API
 */

node *alias_target(const node *alias);
