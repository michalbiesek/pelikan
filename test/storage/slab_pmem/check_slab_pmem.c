#include <storage/slab/item.h>
#include <storage/slab/slab.h>

#include <time/time.h>

#include <cc_bstring.h>
#include <cc_mm.h>

#include <check.h>
#include <stdio.h>
#include <string.h>

/* define for each suite, local scope due to macro visibility rule */
#define SUITE_NAME "slab"
#define DEBUG_LOG  SUITE_NAME ".log"
#define DATAPOOL_PATH "./slab_datapool.pelikan"

slab_options_st options = { SLAB_OPTION(OPTION_INIT) };
slab_metrics_st metrics = { SLAB_METRIC(METRIC_INIT) };

extern delta_time_i max_ttl;

/*
 * utilities
 */
static void
test_setup(void)
{
    option_load_default((struct option *)&options, OPTION_CARDINALITY(options));
    options.slab_datapool.val.vstr = DATAPOOL_PATH;
    slab_setup(&options, &metrics);
}

static void
test_teardown(int un)
{
    slab_teardown();
    if (un)
        unlink(DATAPOOL_PATH);
}

static void
test_reset(int un)
{
    test_teardown(un);
    test_setup();
}

static void
test_assert_insert_basic_entry_exists(struct bstring key)
{
    struct item *it = item_get(&key);
    ck_assert_msg(it != NULL, "item_get could not find key %.*s", key.len, key.data);
    ck_assert_msg(it->is_linked, "item with key %.*s not linked", key.len, key.data);
    ck_assert_msg(!it->in_freeq, "linked item with key %.*s in freeq", key.len, key.data);
    ck_assert_msg(!it->is_raligned, "item with key %.*s is raligned", key.len, key.data);
    ck_assert_int_eq(it->vlen, sizeof("val") - 1);
    ck_assert_int_eq(cc_memcmp("val", item_data(it), sizeof("val") - 1), 0);
    ck_assert_int_eq(it->klen, sizeof("key") - 1);
    ck_assert_int_eq(cc_memcmp("key", item_key(it), sizeof("key") - 1), 0);
}

static void
test_assert_insert_large_entry_exists(struct bstring key)
{
    size_t len;
    char *p;
    struct item *it  = item_get(&key);
    ck_assert_msg(it != NULL, "item_get could not find key %.*s", key.len, key.data);
    ck_assert_msg(it->is_linked, "item with key %.*s not linked", key.len, key.data);
    ck_assert_msg(!it->in_freeq, "linked item with key %.*s in freeq", key.len, key.data);
    ck_assert_msg(!it->is_raligned, "item with key %.*s is raligned", key.len, key.data);
    ck_assert_int_eq(it->vlen, 1000 * KiB);
    ck_assert_int_eq(it->klen, sizeof("key") - 1);
    ck_assert_int_eq(cc_memcmp("key", item_key(it), sizeof("key") - 1), 0);

    for (p = item_data(it), len = it->vlen; len > 0 && *p == 'A'; p++, len--);
    ck_assert_msg(len == 0, "item_data contains wrong value %.*s", (1000 * KiB), item_data(it));
}

static void
test_assert_reserve_backfill_link_exists(struct item *it)
{
    size_t len;
    char *p;

    ck_assert_msg(it->is_linked, "completely backfilled item not linked");
    ck_assert_int_eq(it->vlen, (1000 * KiB));

    for (p = item_data(it), len = it->vlen; len > 0 && *p == 'A'; p++, len--);
    ck_assert_msg(len == 0, "item_data contains wrong value %.*s", (1000 * KiB), item_data(it));
}

/**
 * Tests basic functionality for item_insert with small key/val. Checks that the
 * commands succeed and that the item returned is well-formed.
 */
START_TEST(test_insert_basic)
{
#define KEY "key"
#define VAL "val"
#define MLEN 8
    struct bstring key, val;
    item_rstatus_e status;
    struct item *it;

    key = str2bstr(KEY);
    val = str2bstr(VAL);

    time_update();
    status = item_reserve(&it, &key, &val, val.len, MLEN, INT32_MAX);
    ck_assert_msg(status == ITEM_OK, "item_reserve not OK - return status %d",
            status);
    ck_assert_msg(it != NULL, "item_reserve with key %.*s reserved NULL item",
            key.len, key.data);
    ck_assert_msg(!it->is_linked, "item with key %.*s not linked", key.len,
            key.data);
    ck_assert_msg(!it->in_freeq, "linked item with key %.*s in freeq", key.len,
            key.data);
    ck_assert_msg(!it->is_raligned, "item with key %.*s is raligned", key.len,
            key.data);
    ck_assert_int_eq(it->vlen, sizeof(VAL) - 1);
    ck_assert_int_eq(it->klen, sizeof(KEY) - 1);
    ck_assert_int_eq(item_data(it) - (char *)it, offsetof(struct item, end) +
            item_cas_size() + MLEN + sizeof(KEY) - 1);
    ck_assert_int_eq(cc_memcmp(item_data(it), VAL, val.len), 0);

    item_insert(it, &key);

    test_assert_insert_basic_entry_exists(key);

    test_reset(0);

    test_assert_insert_basic_entry_exists(key);

#undef MLEN
#undef KEY
#undef VAL
}
END_TEST

/**
 * Tests item_insert and item_get for large value (close to 1 MiB). Checks that the commands
 * succeed and that the item returned is well-formed.
 */
START_TEST(test_insert_large)
{
#define KEY "key"
#define VLEN (1000 * KiB)

    struct bstring key, val;
    item_rstatus_e status;
    struct item *it;

    test_reset(1);

    key = str2bstr(KEY);

    val.data = cc_alloc(VLEN);
    cc_memset(val.data, 'A', VLEN);
    val.len = VLEN;

    time_update();
    status = item_reserve(&it, &key, &val, val.len, 0, INT32_MAX);
    free(val.data);
    ck_assert_msg(status == ITEM_OK, "item_reserve not OK - return status %d", status);
    item_insert(it, &key);

    test_assert_insert_large_entry_exists(key);

    test_reset(0);

    test_assert_insert_large_entry_exists(key);

#undef VLEN
#undef KEY
}
END_TEST


START_TEST(test_reserve_backfill_link)
{
#define KEY "key"
#define VLEN (1000 * KiB)

    struct bstring key, val;
    item_rstatus_e status;
    struct item *it;

    test_reset(1);

    key = str2bstr(KEY);

    val.len = VLEN;
    val.data = cc_alloc(val.len);
    cc_memset(val.data, 'A', val.len);

    /* reserve */
    time_update();
    status = item_reserve(&it, &key, &val, val.len, 0, INT32_MAX);
    free(val.data);
    ck_assert_msg(status == ITEM_OK, "item_reserve not OK - return status %d", status);

    /* backfill & link */
    val.len = 0;
    item_backfill(it, &val);
    item_insert(it, &key);
    test_assert_reserve_backfill_link_exists(it);

    test_reset(0);

    test_assert_reserve_backfill_link_exists(it);

#undef VLEN
#undef KEY
}
END_TEST

/**
 * Tests basic append functionality for item_annex.
 */
START_TEST(test_append_basic)
{
#define KEY "key"
#define VAL "val"
#define APPEND "append"
    struct bstring key, val, append;
    item_rstatus_e status;
    struct item *it;

    test_reset(1);

    key = str2bstr(KEY);
    val = str2bstr(VAL);
    append = str2bstr(APPEND);

    time_update();
    status = item_reserve(&it, &key, &val, val.len, 0, INT32_MAX);
    ck_assert_msg(status == ITEM_OK, "item_reserve not OK - return status %d", status);
    item_insert(it, &key);

    it = item_get(&key);
    ck_assert_msg(it != NULL, "item_get could not find key %.*s", key.len, key.data);

    status = item_annex(it, &key, &append, true);
    ck_assert_msg(status == ITEM_OK, "item_append not OK - return status %d", status);

    test_reset(0);

    it = item_get(&key);
    ck_assert_msg(it != NULL, "item_get could not find key %.*s", key.len, key.data);
    ck_assert_msg(it->is_linked, "item with key %.*s not linked", key.len, key.data);
    ck_assert_msg(!it->in_freeq, "linked item with key %.*s in freeq", key.len, key.data);
    ck_assert_msg(!it->is_raligned, "item with key %.*s is raligned", key.len, key.data);
    ck_assert_int_eq(it->vlen, val.len + append.len);
    ck_assert_int_eq(it->klen, sizeof(KEY) - 1);
    ck_assert_int_eq(cc_memcmp(item_data(it), VAL APPEND, val.len + append.len), 0);
#undef KEY
#undef VAL
#undef APPEND
}
END_TEST

/**
 * Tests basic prepend functionality for item_annex.
 */
START_TEST(test_prepend_basic)
{
#define KEY "key"
#define VAL "val"
#define PREPEND "prepend"
    struct bstring key, val, prepend;
    item_rstatus_e status;
    struct item *it;

    test_reset(1);

    key = str2bstr(KEY);
    val = str2bstr(VAL);
    prepend = str2bstr(PREPEND);

    time_update();
    status = item_reserve(&it, &key, &val, val.len, 0, INT32_MAX);
    ck_assert_msg(status == ITEM_OK, "item_reserve not OK - return status %d", status);
    item_insert(it, &key);

    it = item_get(&key);
    ck_assert_msg(it != NULL, "item_get could not find key %.*s", key.len, key.data);

    status = item_annex(it, &key, &prepend, false);
    ck_assert_msg(status == ITEM_OK, "item_prepend not OK - return status %d", status);

    test_reset(0);

    it = item_get(&key);
    ck_assert_msg(it != NULL, "item_get could not find key %.*s", key.len, key.data);
    ck_assert_msg(it->is_linked, "item with key %.*s not linked", key.len, key.data);
    ck_assert_msg(!it->in_freeq, "linked item with key %.*s in freeq", key.len, key.data);
    ck_assert_msg(it->is_raligned, "item with key %.*s is not raligned", key.len, key.data);
    ck_assert_int_eq(it->vlen, val.len + prepend.len);
    ck_assert_int_eq(it->klen, sizeof(KEY) - 1);
    ck_assert_int_eq(cc_memcmp(item_data(it), PREPEND VAL, val.len + prepend.len), 0);
#undef KEY
#undef VAL
#undef PREPEND
}
END_TEST

START_TEST(test_evict_lru_basic)
{
#define MY_SLAB_SIZE 160
#define MY_SLAB_MAXBYTES 160
    /**
     * These are the slabs that will be created with these parameters:
     *
     * slab size 160, slab hdr size 36, item hdr size 40, item chunk size44, total memory 320
     * class   1: items       2  size      48  data       8  slack      28
     * class   2: items       1  size     120  data      80  slack       4
     *
     * If we use 8 bytes of key+value, it will use the class 1 that can fit
     * two elements. The third one will cause a full slab eviction.
     *
     **/
#define KEY_LENGTH 2
#define VALUE_LENGTH 8
#define NUM_ITEMS 2

    size_t i;
    struct bstring key[NUM_ITEMS + 1] = {
        {KEY_LENGTH, "aa"},
        {KEY_LENGTH, "bb"},
        {KEY_LENGTH, "cc"},
    };
    struct bstring val[NUM_ITEMS + 1] = {
        {VALUE_LENGTH, "aaaaaaaa"},
        {VALUE_LENGTH, "bbbbbbbb"},
        {VALUE_LENGTH, "cccccccc"},
    };
    item_rstatus_e status;
    struct item *it;

    option_load_default((struct option *)&options, OPTION_CARDINALITY(options));
    options.slab_datapool.val.vstr = DATAPOOL_PATH;
    options.slab_size.val.vuint = MY_SLAB_SIZE;
    options.slab_mem.val.vuint = MY_SLAB_MAXBYTES;
    options.slab_evict_opt.val.vuint = EVICT_CS;
    options.slab_item_max.val.vuint = MY_SLAB_SIZE - SLAB_HDR_SIZE;

    test_teardown(1);
    slab_setup(&options, &metrics);

    for (i = 0; i < NUM_ITEMS + 1; i++) {
        time_update();
        status = item_reserve(&it, &key[i], &val[i], val[i].len, 0, INT32_MAX);
        ck_assert_msg(status == ITEM_OK, "item_reserve not OK - return status %d", status);
        item_insert(it, &key[i]);
        ck_assert_msg(item_get(&key[i]) != NULL, "item %lu not found", i);
    }

    ck_assert_msg(item_get(&key[0]) == NULL,
        "item 0 found, expected to be evicted");
    ck_assert_msg(item_get(&key[1]) == NULL,
        "item 1 found, expected to be evicted");
    ck_assert_msg(item_get(&key[2]) != NULL,
        "item 2 not found");

#undef KEY_LENGTH
#undef VALUE_LENGTH
#undef NUM_ITEMS
#undef MY_SLAB_SIZE
#undef MY_SLAB_MAXBYTES
}
END_TEST

/*
 * test suite
 */
static Suite *
slab_suite(void)
{
    Suite *s = suite_create(SUITE_NAME);

    /* basic item */
    TCase *tc_item = tcase_create("item api");
    suite_add_tcase(s, tc_item);

    tcase_add_test(tc_item, test_insert_basic);
    tcase_add_test(tc_item, test_insert_large);
    tcase_add_test(tc_item, test_reserve_backfill_link);
    tcase_add_test(tc_item, test_append_basic);
    tcase_add_test(tc_item, test_prepend_basic);

    TCase *tc_slab = tcase_create("slab api");
    suite_add_tcase(s, tc_slab);
    tcase_add_test(tc_slab, test_evict_lru_basic);
    return s;
}

int
main(void)
{
    int nfail;

    /* setup */
    test_setup();

    Suite *suite = slab_suite();
    SRunner *srunner = srunner_create(suite);
    srunner_set_fork_status(srunner, CK_NOFORK);
    srunner_set_log(srunner, DEBUG_LOG);
    srunner_run_all(srunner, CK_ENV); /* set CK_VEBOSITY in ENV to customize */
    nfail = srunner_ntests_failed(srunner);
    srunner_free(srunner);

    /* teardown */
    test_teardown(1);

    return (nfail == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
