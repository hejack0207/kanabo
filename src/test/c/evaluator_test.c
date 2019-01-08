#include "test.h"
#include "test_document.h"
#include "test_nodelist.h"

#include "evaluator.h"
#include "parser.h"
#include "loader.h"


static DocumentSet *model_fixture = NULL;

#define assert_evaluator_failure(CONTEXT, CODE)                         \
    do                                                                  \
    {                                                                   \
        assert_nothing((CONTEXT));                                      \
        assert_int_eq((CODE), (CONTEXT).error);                         \
        log_debug("evaluator test", "received expected failure message: '%s'", evaluator_strerror((CODE))); \
    } while(0)

static DocumentSet *must_load(const char *filename)
{
    assert_not_null(filename);
    Maybe(Input) input = make_input_from_file(filename);
    assert_just(input);

    Maybe(DocumentSet) yaml = load_yaml(from_just(input), DUPE_FAIL);
    assert_just(yaml);

    return from_just(yaml);
}

static void inventory_setup(void)
{
    model_fixture = must_load("test-resources/inventory.json");
}

static void invoice_setup(void)
{
    model_fixture = must_load("test-resources/invoice.yaml");
}

static void evaluator_teardown(void)
{
    dispose_document_set(model_fixture);
    model_fixture = NULL;
}

START_TEST (null_model)
{
    char *expression = "foo";

    Maybe(JsonPath) path = parse(expression);
    assert_just(path);

    Maybe(Nodelist) list = evaluate(NULL, from_just(path));
    assert_evaluator_failure(list, ERR_MODEL_IS_NULL);

    dispose_path(from_just(path));
}
END_TEST

START_TEST (null_path)
{
    DocumentSet *bad_model = make_document_set();

    Maybe(Nodelist) list = evaluate(bad_model, NULL);
    assert_evaluator_failure(list, ERR_PATH_IS_NULL);

    dispose_document_set(bad_model);
}
END_TEST

START_TEST (null_document)
{
    char *expression = "foo";

    Maybe(JsonPath) path = parse(expression);
    assert_just(path);

    DocumentSet *bad_model = make_document_set();

    Maybe(Nodelist) list = evaluate(bad_model, from_just(path));
    assert_evaluator_failure(list, ERR_NO_DOCUMENT_IN_MODEL);

    dispose_document_set(bad_model);
    dispose_path(from_just(path));
}
END_TEST

START_TEST (null_document_root)
{
    char *expression = "foo";

    Maybe(JsonPath) path = parse(expression);
    assert_just(path);

    DocumentSet *bad_model = make_document_set();
    Document *doc = make_document_node();
    assert_not_null(doc);
    document_set_add(bad_model, doc);

    Maybe(Nodelist) list = evaluate(bad_model, from_just(path));
    assert_evaluator_failure(list, ERR_NO_ROOT_IN_DOCUMENT);

    dispose_document_set(bad_model);
    dispose_path(from_just(path));
}
END_TEST

START_TEST (relative_path)
{
    char *expression = "foo";

    Maybe(JsonPath) path = parse(expression);
    assert_just(path);

    DocumentSet *bad_model = make_document_set();
    Mapping *root = make_mapping_node();
    Document *doc = make_document_node();
    document_set_root(doc, node(root));
    document_set_add(bad_model, doc);

    Maybe(Nodelist) list = evaluate(bad_model, from_just(path));
    assert_evaluator_failure(list, ERR_PATH_IS_NOT_ABSOLUTE);

    dispose_document_set(bad_model);
    dispose_path(from_just(path));
}
END_TEST


START_TEST (empty_path)
{
    JsonPath *path = (JsonPath *)calloc(1, sizeof(JsonPath));
    assert_not_null(path);
    path->kind = ABSOLUTE_PATH;
    path->steps = NULL;

    DocumentSet *bad_model = make_document_set();
    Mapping *root = make_mapping_node();
    Document *doc = make_document_node();
    document_set_root(doc, node(root));
    document_set_add(bad_model, doc);

    Maybe(Nodelist) list = evaluate(bad_model, path);
    assert_evaluator_failure(list, ERR_PATH_IS_EMPTY);

    dispose_document_set(bad_model);
    dispose_path(path);
}
END_TEST

static Nodelist *evaluate_expression(const char *expression)
{
    Maybe(JsonPath) path = parse(expression);
    assert_just(path);

    Maybe(Nodelist) list = evaluate(model_fixture, from_just(path));
    assert_just(list);

    dispose_path(from_just(path));

    return from_just(list);
}

START_TEST (dollar_only)
{
    Nodelist *list = evaluate_expression("$");

    assert_nodelist_length(list, 1);
    assert_node_kind(nodelist_get(list, 0), MAPPING);
    assert_node_size(nodelist_get(list, 0), 1);
    assert_mapping_has_key(nodelist_get(list, 0), "store");

    nodelist_free(list);
}
END_TEST

START_TEST (single_name_step)
{
    Nodelist *list = evaluate_expression("$.store");

    assert_nodelist_length(list, 1);
    Node *store = nodelist_get(list, 0);
    assert_not_null(store);

    assert_node_kind(store, MAPPING);
    assert_node_size(store, 2);
    assert_mapping_has_key(mapping(store), "book");
    assert_mapping_has_key(mapping(store), "bicycle");

    nodelist_free(list);
}
END_TEST

START_TEST (simple_recursive_step)
{
    Nodelist *list = evaluate_expression("$..author");

    assert_nodelist_length(list, 5);

    assert_node_kind(nodelist_get(list, 0), SCALAR);
    assert_node_kind(nodelist_get(list, 1), SCALAR);
    assert_node_kind(nodelist_get(list, 2), SCALAR);
    assert_node_kind(nodelist_get(list, 3), SCALAR);

    nodelist_free(list);
}
END_TEST

START_TEST (compound_recursive_step)
{
    Nodelist *list = evaluate_expression("$.store..price");

    assert_nodelist_length(list, 6);

    assert_scalar_kind(nodelist_get(list, 0), SCALAR_REAL);
    assert_scalar_kind(nodelist_get(list, 1), SCALAR_REAL);
    assert_scalar_kind(nodelist_get(list, 2), SCALAR_REAL);
    assert_scalar_kind(nodelist_get(list, 3), SCALAR_REAL);
    assert_scalar_kind(nodelist_get(list, 4), SCALAR_REAL);
    assert_scalar_kind(nodelist_get(list, 5), SCALAR_REAL);

    nodelist_free(list);
}
END_TEST

START_TEST (long_path)
{
    Nodelist *list = evaluate_expression("$.store.bicycle.color");

    assert_nodelist_length(list, 1);
    Node *color = nodelist_get(list, 0);
    assert_not_null(color);

    assert_node_kind(color, SCALAR);
    assert_scalar_value((color), "red");

    nodelist_free(list);
}
END_TEST

START_TEST (wildcard)
{
    Nodelist *list = evaluate_expression("$.store.*");

    assert_nodelist_length(list, 6);

    assert_node_kind(nodelist_get(list, 0), MAPPING);
    assert_node_kind(nodelist_get(list, 1), MAPPING);
    assert_node_kind(nodelist_get(list, 2), MAPPING);
    assert_node_kind(nodelist_get(list, 3), MAPPING);
    assert_node_kind(nodelist_get(list, 4), MAPPING);

    nodelist_free(list);
}
END_TEST

START_TEST (recursive_wildcard)
{
    Nodelist *list = evaluate_expression("$..*");

    assert_nodelist_length(list, 34);

    assert_node_kind(nodelist_get(list, 0), MAPPING);

    nodelist_free(list);
}
END_TEST

START_TEST (object_test)
{
    Nodelist *list = evaluate_expression("$.store.object()");

    assert_nodelist_length(list, 1);

    Node *value = nodelist_get(list, 0);
    assert_not_null(value);

    assert_node_kind(value, MAPPING);
    assert_mapping_has_key(mapping(value), "book");
    assert_mapping_has_key(mapping(value), "bicycle");

    nodelist_free(list);
}
END_TEST

START_TEST (array_test)
{
    Nodelist *list = evaluate_expression("$.store.book.array()");

    assert_nodelist_length(list, 1);

    Node *value = nodelist_get(list, 0);
    assert_not_null(value);
    assert_node_kind(value, SEQUENCE);
    assert_node_size(value, 5);

    nodelist_free(list);
}
END_TEST

START_TEST (number_test)
{
    Nodelist *list = evaluate_expression("$.store.book[*].price.number()");

    assert_nodelist_length(list, 5);

    assert_scalar_value(nodelist_get(list, 0), "8.95");
    assert_scalar_value(nodelist_get(list, 1), "12.99");
    assert_scalar_value(nodelist_get(list, 2), "8.99");
    assert_scalar_value(nodelist_get(list, 3), "22.99");

    nodelist_free(list);
}
END_TEST

START_TEST (wildcard_predicate)
{
    Nodelist *list = evaluate_expression("$.store.book[*].author");

    assert_nodelist_length(list, 5);

    assert_node_kind(nodelist_get(list, 0), SCALAR);
    assert_node_kind(nodelist_get(list, 1), SCALAR);
    assert_node_kind(nodelist_get(list, 2), SCALAR);
    assert_node_kind(nodelist_get(list, 3), SCALAR);

    nodelist_free(list);
}
END_TEST

START_TEST (wildcard_predicate_on_mapping)
{
    Nodelist *list = evaluate_expression("$.store.bicycle[*].color");

    assert_nodelist_length(list, 1);

    Node *s = nodelist_get(list, 0);
    assert_node_kind(s, SCALAR);
    assert_scalar_value((s), "red");

    nodelist_free(list);
}
END_TEST

START_TEST (wildcard_predicate_on_scalar)
{
    Nodelist *list = evaluate_expression("$.store.bicycle.color[*]");

    assert_nodelist_length(list, 1);

    Node *s = nodelist_get(list, 0);
    assert_node_kind(s, SCALAR);
    assert_scalar_value((s), "red");

    nodelist_free(list);
}
END_TEST

START_TEST (subscript_predicate)
{
    Nodelist *list = evaluate_expression("$.store.book[2]");

    assert_nodelist_length(list, 1);

    Node *book = nodelist_get(list, 0);
    assert_node_kind(book, MAPPING);

    Scalar *key = make_scalar_string("author");
    Node *author = mapping_get(mapping(book), key);
    dispose_node(key);
    assert_not_null(author);
    assert_scalar_value((author), "Herman Melville");

    nodelist_free(list);
}
END_TEST

START_TEST (recursive_subscript_predicate)
{
    Nodelist *list = evaluate_expression("$..book[2]");

    assert_nodelist_length(list, 1);

    Node *book = nodelist_get(list, 0);
    assert_node_kind(book, MAPPING);

    Scalar *key = make_scalar_string("author");
    Node *author = mapping_get(mapping(book), key);
    dispose_node(key);
    assert_not_null(author);
    assert_scalar_value((author), "Herman Melville");

    nodelist_free(list);
}
END_TEST

START_TEST (slice_predicate)
{
    Nodelist *list = evaluate_expression("$.store.book[:2]");

    assert_nodelist_length(list, 2);

    Node *book = nodelist_get(list, 0);
    assert_node_kind(book, MAPPING);

    Scalar *key = make_scalar_string("author");

    Node *author = mapping_get(mapping(book), key);
    assert_not_null(author);
    assert_scalar_value((author), "Nigel Rees");

    book = nodelist_get(list, 1);
    assert_node_kind(book, MAPPING);

    author = mapping_get(mapping(book), key);
    assert_not_null(author);
    assert_scalar_value((author), "Evelyn Waugh");

    dispose_node(key);
    nodelist_free(list);
}
END_TEST

START_TEST (recursive_slice_predicate)
{
    Nodelist *list = evaluate_expression("$..book[:2]");

    assert_nodelist_length(list, 2);

    Node *book = nodelist_get(list, 0);
    assert_node_kind(book, MAPPING);

    Scalar *key = make_scalar_string("author");

    Node *author = mapping_get(mapping(book), key);
    assert_not_null(author);
    assert_scalar_value((author), "Nigel Rees");

    book = nodelist_get(list, 1);
    assert_node_kind(book, MAPPING);

    author = mapping_get(mapping(book), key);
    assert_not_null(author);
    assert_scalar_value((author), "Evelyn Waugh");

    dispose_node(key);
    nodelist_free(list);
}
END_TEST

START_TEST (slice_predicate_with_step)
{
    Nodelist *list = evaluate_expression("$.store.book[:2:2]");

    assert_nodelist_length(list, 1);

    Node *book = nodelist_get(list, 0);
    assert_node_kind(book, MAPPING);

    Scalar *key = make_scalar_string("author");
    Node *author = mapping_get(mapping(book), key);
    dispose_node(key);
    assert_not_null(author);
    assert_scalar_value((author), "Nigel Rees");

    nodelist_free(list);
}
END_TEST

START_TEST (slice_predicate_negative_from)
{
    Nodelist *list = evaluate_expression("$.store.book[-1:]");

    assert_nodelist_length(list, 1);

    Node *book = nodelist_get(list, 0);
    assert_node_kind(book, MAPPING);

    Scalar *key = make_scalar_string("author");
    Node *author = mapping_get(mapping(book), key);
    dispose_node(key);
    assert_not_null(author);
    assert_scalar_value((author), "夏目漱石 (NATSUME Sōseki)");

    nodelist_free(list);
}
END_TEST

START_TEST (recursive_slice_predicate_negative_from)
{
    Nodelist *list = evaluate_expression("$..book[-1:]");

    assert_nodelist_length(list, 1);

    Node *book = nodelist_get(list, 0);
    assert_node_kind(book, MAPPING);

    Scalar *key = make_scalar_string("author");
    Node *author = mapping_get(mapping(book), key);
    dispose_node(key);
    assert_not_null(author);
    assert_scalar_value((author), "夏目漱石 (NATSUME Sōseki)");

    nodelist_free(list);
}
END_TEST

START_TEST (slice_predicate_copy)
{
    Nodelist *list = evaluate_expression("$.store.book[::]");

    assert_nodelist_length(list, 5);

    Scalar *key = make_scalar_string("author");

    Node *value;
    value = mapping_get(nodelist_get(list, 0), key);
    assert_scalar_value((value), "Nigel Rees");
    value = mapping_get(nodelist_get(list, 1), key);
    assert_scalar_value((value), "Evelyn Waugh");
    value = mapping_get(nodelist_get(list, 2), key);
    assert_scalar_value((value), "Herman Melville");
    value = mapping_get(nodelist_get(list, 3), key);
    assert_scalar_value((value), "J. R. R. Tolkien");

    nodelist_free(list);
}
END_TEST

START_TEST (slice_predicate_reverse)
{
    Nodelist *list = evaluate_expression("$.store.book[::-1]");

    assert_nodelist_length(list, 5);

    Scalar *key = make_scalar_string("author");

    Node *value;
    value = mapping_get(nodelist_get(list, 0), key);
    assert_scalar_value((value), "夏目漱石 (NATSUME Sōseki)");
    value = mapping_get(nodelist_get(list, 1), key);
    assert_scalar_value((value), "J. R. R. Tolkien");
    value = mapping_get(nodelist_get(list, 2), key);
    assert_scalar_value((value), "Herman Melville");
    value = mapping_get(nodelist_get(list, 3), key);
    assert_scalar_value((value), "Evelyn Waugh");
    value = mapping_get(nodelist_get(list, 4), key);
    assert_scalar_value((value), "Nigel Rees");

    dispose_node(key);
    nodelist_free(list);
}
END_TEST

START_TEST (name_alias)
{
    Nodelist *list = evaluate_expression("$.payment.billing-address.name");

    assert_nodelist_length(list, 1);
    assert_scalar_value(nodelist_get(list, 0), "Ramond Hessel");

    nodelist_free(list);
}
END_TEST

START_TEST (type_alias)
{
    Nodelist *list = evaluate_expression("$.shipments[0].*.number()");

    assert_nodelist_length(list, 1);
    assert_scalar_value(nodelist_get(list, 0), "237.23");

    nodelist_free(list);
}
END_TEST

START_TEST (greedy_wildcard_alias)
{
    Nodelist *list = evaluate_expression("$.shipments[0].items.*");

    assert_nodelist_length(list, 2);

    Scalar *key = make_scalar_string("isbn");

    Node *zero = nodelist_get(list, 0);
    assert_node_kind(zero, MAPPING);
    Node *zero_value = mapping_get(mapping(zero), key);
    assert_scalar_value((zero_value), "1428312250");

    Node *one = nodelist_get(list, 1);
    assert_node_kind(one, MAPPING);
    Node *one_value = mapping_get(mapping(one),key);
    assert_scalar_value((one_value), "0323073867");

    dispose_node(key);
    nodelist_free(list);
}
END_TEST

START_TEST (recursive_alias)
{
    Nodelist *list = evaluate_expression("$.shipments..isbn");

    assert_nodelist_length(list, 2);

    assert_scalar_value(nodelist_get(list, 0), "1428312250");
    assert_scalar_value(nodelist_get(list, 1), "0323073867");

    nodelist_free(list);
}
END_TEST

START_TEST (wildcard_predicate_alias)
{
    Nodelist *list = evaluate_expression("$.shipments[0].items[*].price");

    assert_nodelist_length(list, 2);

    assert_scalar_value(nodelist_get(list, 0), "135.48");
    assert_scalar_value(nodelist_get(list, 1), "84.18");

    nodelist_free(list);
}
END_TEST

START_TEST (recursive_wildcard_alias)
{
    Nodelist *list = evaluate_expression("$.shipments[0].items..*");

    assert_nodelist_length(list, 15);

    nodelist_free(list);
}
END_TEST

Suite *evaluator_suite(void)
{
    TCase *bad_input_case = tcase_create("bad input");
    tcase_add_test(bad_input_case, null_model);
    tcase_add_test(bad_input_case, null_path);
    tcase_add_test(bad_input_case, null_document);
    tcase_add_test(bad_input_case, null_document_root);
    tcase_add_test(bad_input_case, relative_path);
    tcase_add_test(bad_input_case, empty_path);

    TCase *basic_case = tcase_create("basic");
    tcase_add_unchecked_fixture(basic_case, inventory_setup, evaluator_teardown);
    tcase_add_test(basic_case, dollar_only);
    tcase_add_test(basic_case, single_name_step);
    tcase_add_test(basic_case, long_path);
    tcase_add_test(basic_case, wildcard);
    tcase_add_test(basic_case, object_test);
    tcase_add_test(basic_case, array_test);
    tcase_add_test(basic_case, number_test);

    TCase *predicate_case = tcase_create("predicate");
    tcase_add_unchecked_fixture(predicate_case, inventory_setup, evaluator_teardown);
    tcase_add_test(predicate_case, wildcard_predicate);
    tcase_add_test(predicate_case, wildcard_predicate_on_mapping);
    tcase_add_test(predicate_case, wildcard_predicate_on_scalar);
    tcase_add_test(predicate_case, subscript_predicate);
    tcase_add_test(predicate_case, slice_predicate);
    tcase_add_test(predicate_case, subscript_predicate);
    tcase_add_test(predicate_case, slice_predicate_with_step);
    tcase_add_test(predicate_case, slice_predicate_negative_from);
    tcase_add_test(predicate_case, slice_predicate_copy);
    tcase_add_test(predicate_case, slice_predicate_reverse);

    TCase *recursive_case = tcase_create("recursive");
    tcase_add_unchecked_fixture(recursive_case, inventory_setup, evaluator_teardown);
    tcase_add_test(recursive_case, simple_recursive_step);
    tcase_add_test(recursive_case, compound_recursive_step);
    tcase_add_test(recursive_case, recursive_slice_predicate);
    tcase_add_test(recursive_case, recursive_subscript_predicate);
    tcase_add_test(recursive_case, recursive_slice_predicate_negative_from);
    tcase_add_test(recursive_case, recursive_wildcard);

    TCase *alias_case = tcase_create("alias");
    tcase_add_unchecked_fixture(alias_case, invoice_setup, evaluator_teardown);
    tcase_add_test(alias_case, name_alias);
    tcase_add_test(alias_case, type_alias);
    tcase_add_test(alias_case, greedy_wildcard_alias);
    tcase_add_test(alias_case, recursive_alias);
    tcase_add_test(alias_case, wildcard_predicate_alias);
    tcase_add_test(alias_case, recursive_wildcard_alias);

    Suite *evaluator = suite_create("Evaluator");
    suite_add_tcase(evaluator, bad_input_case);
    suite_add_tcase(evaluator, basic_case);
    suite_add_tcase(evaluator, predicate_case);
    suite_add_tcase(evaluator, recursive_case);
    suite_add_tcase(evaluator, alias_case);

    return evaluator;
}
