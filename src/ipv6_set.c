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

#include <bdd.h>
#include <ipset/ipset.h>


void
ipset_init_ipv6_set(ipv6_set_t *set)
{
    /*
     * The set starts empty, so every value assignment should yield
     * false.
     */

    set->set_bdd = bdd_addref(bddfalse);
}


ipv6_set_t *
ipset_new_ipv6_set()
{
    ipv6_set_t  *result = NULL;

    /*
     * Try to allocate a new set.
     */

    result = (ipv6_set_t *) malloc(sizeof(ipv6_set_t));
    if (result == NULL)
        return NULL;

    /*
     * If that worked, initialize and return the set.
     */

    ipset_init_ipv6_set(result);
    return result;
}


void
ipset_done_ipv6_set(ipv6_set_t *set)
{
    bdd_delref(set->set_bdd);
}


void
ipset_free_ipv6_set(ipv6_set_t *set)
{
    ipset_done_ipv6_set(set);
    free(set);
}
