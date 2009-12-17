/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2009, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the LICENSE.txt file in this distribution for license
 * details.
 * ----------------------------------------------------------------------
 */

#include <stdlib.h>

#include <check.h>

#include <ipset/ipset.h>


/*-----------------------------------------------------------------------
 * IPv4 tests
 */

START_TEST(test_ipv4_starts_empty)
{
    ipv4_set_t  set;

    ipset_ipv4_init(&set);
    fail_unless(ipset_ipv4_is_empty(&set),
                "IPv4 set should start empty");
    ipset_ipv4_done(&set);
}
END_TEST


/*-----------------------------------------------------------------------
 * IPv6 tests
 */

START_TEST(test_ipv6_starts_empty)
{
    ipv6_set_t  set;

    ipset_ipv6_init(&set);
    fail_unless(ipset_ipv6_is_empty(&set),
                "IPv6 set should start empty");
    ipset_ipv6_done(&set);
}
END_TEST


/*-----------------------------------------------------------------------
 * Testing harness
 */

Suite *
ipset_suite()
{
    Suite  *s = suite_create("ipset");

    TCase  *tc_ipv4 = tcase_create("ipv4");
    tcase_add_test(tc_ipv4, test_ipv4_starts_empty);
    suite_add_tcase(s, tc_ipv4);

    TCase  *tc_ipv6 = tcase_create("ipv6");
    tcase_add_test(tc_ipv6, test_ipv6_starts_empty);
    suite_add_tcase(s, tc_ipv6);

    return s;
}


int
main(int argc, const char **argv)
{
    int  number_failed;
    Suite  *suite = ipset_suite();
    SRunner  *runner = srunner_create(suite);

    ipset_init();

    srunner_run_all(runner, CK_NORMAL);
    number_failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (number_failed == 0)? EXIT_SUCCESS: EXIT_FAILURE;
}
