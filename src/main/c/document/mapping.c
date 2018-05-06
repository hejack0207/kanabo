#include "document.h"
#include "conditions.h"

struct context_adapter_s
{
    mapping_iterator iterator;
    void *context;
};

typedef struct context_adapter_s context_adapter;

static bool mapping_equals(const Node *one, const Node *two)
{
    return hashtable_equals(((const Mapping *)one)->values,
                            ((const Mapping *)two)->values,
                            node_comparitor);
}

static size_t mapping_size(const Node *self)
{
    return hashtable_size(((const Mapping *)self)->values);
}

static bool mapping_freedom_iterator(void *key, void *value, void *context __attribute__((unused)))
{
    node_free(key);
    node_free(value);

    return true;
}

static void mapping_free(Node *value)
{
    Mapping *map = (Mapping *)value;
    if(NULL == map->values)
    {
        return;
    }

    hashtable_iterate(map->values, mapping_freedom_iterator, NULL);
    hashtable_free(map->values);
    map->values = NULL;
}

static hashcode scalar_hash(const void *key)
{
    const Scalar *value = (const Scalar *)key;
    return shift_add_xor_string_buffer_hash(scalar_value(value), node_size(key));
}

static bool scalar_comparitor(const void *one, const void *two)
{
    return node_equals(const_node(one), const_node(two));
}

static const struct vtable_s mapping_vtable = 
{
    mapping_free,
    mapping_size,
    mapping_equals
};

Mapping *make_mapping_node(void)
{
    Mapping *self = calloc(1, sizeof(Mapping));
    if(NULL != self)
    {
        node_init(self, MAPPING);
        self->values = make_hashtable_with_function(scalar_comparitor, scalar_hash);
        if(NULL == self->values)
        {
            free(self);
            self = NULL;
            return NULL;
        }
        self->base.vtable = &mapping_vtable;
    }

    return self;
}

Node *mapping_get(const Mapping *self, uint8_t *value, size_t length)
{
    PRECOND_NONNULL_ELSE_NULL(self, value);
    PRECOND_ELSE_NULL(0 < length);

    Scalar *key = make_scalar_node(value, length, SCALAR_STRING);
    Node *result = hashtable_get(self->values, key);
    node_free(key);

    return result;
}

bool mapping_contains(const Mapping *self, uint8_t *value, size_t length)
{
    PRECOND_NONNULL_ELSE_FALSE(self, value);
    PRECOND_ELSE_FALSE(0 < length);

    Scalar *key = make_scalar_node(value, length, SCALAR_STRING);
    bool result = hashtable_contains(self->values, key);
    node_free(key);

    return result;
}

static bool mapping_iterator_adpater(void *key, void *value, void *context)
{
    context_adapter *adapter = (context_adapter *)context;
    return adapter->iterator(node(key), node(value), adapter->context);
}

bool mapping_iterate(const Mapping *self, mapping_iterator iterator, void *context)
{
    PRECOND_NONNULL_ELSE_FALSE(self, iterator);

    context_adapter adapter = {.iterator=iterator, .context=context};
    return hashtable_iterate(self->values, mapping_iterator_adpater, &adapter);
}

bool mapping_put(Mapping *self, uint8_t *scalar, size_t length, Node *value)
{
    PRECOND_NONNULL_ELSE_FALSE(self, scalar, value);

    Scalar *key = make_scalar_node(scalar, length, SCALAR_STRING);

    errno = 0;
    hashtable_put(self->values, key, value);
    if(0 == errno)
    {
        value->parent = node(self);
    }

    return 0 == errno;
}

