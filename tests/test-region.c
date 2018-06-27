/* -*- Mode: C; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
   Copyright (C) 2009 Red Hat, Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, see <http://www.gnu.org/licenses/>.
*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <spice/macros.h>

#include <common/region.h>

static int slow_region_test(const QRegion *rgn, const QRegion *other_rgn, int query)
{
    pixman_region32_t intersection;
    int res;

    pixman_region32_init(&intersection);
    pixman_region32_intersect(&intersection,
                              (pixman_region32_t *)rgn,
                              (pixman_region32_t *)other_rgn);

    res = 0;

    if (query & REGION_TEST_SHARED &&
        pixman_region32_not_empty(&intersection)) {
        res |= REGION_TEST_SHARED;
    }

    if (query & REGION_TEST_LEFT_EXCLUSIVE &&
        !pixman_region32_equal(&intersection, (pixman_region32_t *)rgn)) {
        res |= REGION_TEST_LEFT_EXCLUSIVE;
    }

    if (query & REGION_TEST_RIGHT_EXCLUSIVE &&
        !pixman_region32_equal(&intersection, (pixman_region32_t *)other_rgn)) {
        res |= REGION_TEST_RIGHT_EXCLUSIVE;
    }

    pixman_region32_fini(&intersection);

    return res;
}


static int rect_is_valid(const SpiceRect *r)
{
    if (r->top > r->bottom || r->left > r->right) {
        printf("%s: invalid rect\n", __FUNCTION__);
        return FALSE;
    }
    return TRUE;
}

static void rect_set(SpiceRect *r, int32_t top, int32_t left, int32_t bottom, int32_t right)
{
    r->top = top;
    r->left = left;
    r->bottom = bottom;
    r->right = right;
    spice_assert(rect_is_valid(r));
}

static void random_region(QRegion *reg)
{
    int i;
    int num_rects;
    int x, y, w, h;
    SpiceRect _r;
    SpiceRect *r = &_r;

    region_clear(reg);

    num_rects = rand() % 20;
    for (i = 0; i < num_rects; i++) {
        x = rand()%100;
        y = rand()%100;
        w = rand()%100;
        h = rand()%100;
        rect_set(r,
                 x, y,
                 x+w, y+h);
        region_add(reg, r);
    }
}

static void test(const QRegion *r1, const QRegion *r2, int *expected)
{
    printf("r1 is_empty %s [%s]\n",
           region_is_empty(r1) ? "TRUE" : "FALSE",
           (region_is_empty(r1) == *(expected++)) ? "OK" : "ERR");
    printf("r2 is_empty %s [%s]\n",
           region_is_empty(r2) ? "TRUE" : "FALSE",
           (region_is_empty(r2) == *(expected++)) ? "OK" : "ERR");
    printf("is_equal %s [%s]\n",
           region_is_equal(r1, r2) ? "TRUE" : "FALSE",
           (region_is_equal(r1, r2) == *(expected++)) ? "OK" : "ERR");
    printf("intersects %s [%s]\n",
           region_intersects(r1, r2) ? "TRUE" : "FALSE",
           (region_intersects(r1, r2) == *(expected++)) ? "OK" : "ERR");
    printf("contains %s [%s]\n",
           region_contains(r1, r2) ? "TRUE" : "FALSE",
           (region_contains(r1, r2) == *(expected++)) ? "OK" : "ERR");
}

enum {
    EXPECT_R1_EMPTY,
    EXPECT_R2_EMPTY,
    EXPECT_EQUAL,
    EXPECT_SECT,
    EXPECT_CONT,
};

int main(void)
{
    QRegion _r1, _r2, _r3;
    QRegion *r1 = &_r1;
    QRegion *r2 = &_r2;
    QRegion *r3 = &_r3;
    SpiceRect _r;
    SpiceRect *r = &_r;
    int expected[5];
    int i, j;

    region_init(r1);
    region_init(r2);

    printf("dump r1 empty rgn [%s]\n", region_is_valid(r1) ? "VALID" : "INVALID");
    region_dump(r1, "");
    expected[EXPECT_R1_EMPTY] = TRUE;
    expected[EXPECT_R2_EMPTY] = TRUE;
    expected[EXPECT_EQUAL] = TRUE;
    expected[EXPECT_SECT] = FALSE;
    expected[EXPECT_CONT] = TRUE;
    test(r1, r2, expected);
    printf("\n");

    region_clone(r3, r1);
    printf("dump r3 clone rgn [%s]\n", region_is_valid(r3) ? "VALID" : "INVALID");
    region_dump(r3, "");
    expected[EXPECT_R1_EMPTY] = TRUE;
    expected[EXPECT_R2_EMPTY] = TRUE;
    expected[EXPECT_EQUAL] = TRUE;
    expected[EXPECT_SECT] = FALSE;
    expected[EXPECT_CONT] = TRUE;
    test(r1, r3, expected);
    region_destroy(r3);
    printf("\n");

    rect_set(r, 0, 0, 100, 100);
    region_add(r1, r);
    printf("dump r1 [%s]\n", region_is_valid(r1) ? "VALID" : "INVALID");
    region_dump(r1, "");
    expected[EXPECT_R1_EMPTY] = FALSE;
    expected[EXPECT_R2_EMPTY] = TRUE;
    expected[EXPECT_EQUAL] = FALSE;
    expected[EXPECT_SECT] = FALSE;
    expected[EXPECT_CONT] = TRUE;
    test(r1, r2, expected);
    printf("\n");

    region_clear(r1);
    rect_set(r, 0, 0, 0, 0);
    region_add(r1, r);
    printf("dump r1 [%s]\n", region_is_valid(r1) ? "VALID" : "INVALID");
    region_dump(r1, "");
    expected[EXPECT_R1_EMPTY] = TRUE;
    expected[EXPECT_R2_EMPTY] = TRUE;
    expected[EXPECT_EQUAL] = TRUE;
    expected[EXPECT_SECT] = FALSE;
    expected[EXPECT_CONT] = TRUE;
    test(r1, r2, expected);
    printf("\n");

    rect_set(r, -100, -100, 0, 0);
    region_add(r1, r);
    printf("dump r1 [%s]\n", region_is_valid(r1) ? "VALID" : "INVALID");
    region_dump(r1, "");
    expected[EXPECT_R1_EMPTY] = FALSE;
    expected[EXPECT_R2_EMPTY] = TRUE;
    expected[EXPECT_EQUAL] = FALSE;
    expected[EXPECT_SECT] = FALSE;
    expected[EXPECT_CONT] = TRUE;
    test(r1, r2, expected);
    printf("\n");

    region_clear(r1);
    rect_set(r, -100, -100, 100, 100);
    region_add(r1, r);
    printf("dump r1 [%s]\n", region_is_valid(r1) ? "VALID" : "INVALID");
    region_dump(r1, "");
    expected[EXPECT_R1_EMPTY] = FALSE;
    expected[EXPECT_R2_EMPTY] = TRUE;
    expected[EXPECT_EQUAL] = FALSE;
    expected[EXPECT_SECT] = FALSE;
    expected[EXPECT_CONT] = TRUE;
    test(r1, r2, expected);
    printf("\n");


    region_clear(r1);
    region_clear(r2);

    rect_set(r, 100, 100, 200, 200);
    region_add(r1, r);
    printf("dump r1 [%s]\n", region_is_valid(r1) ? "VALID" : "INVALID");
    region_dump(r1, "");
    expected[EXPECT_R1_EMPTY] = FALSE;
    expected[EXPECT_R2_EMPTY] = TRUE;
    expected[EXPECT_EQUAL] = FALSE;
    expected[EXPECT_SECT] = FALSE;
    expected[EXPECT_CONT] = TRUE;
    test(r1, r2, expected);
    printf("\n");

    rect_set(r, 300, 300, 400, 400);
    region_add(r1, r);
    printf("dump r1 [%s]\n", region_is_valid(r1) ? "VALID" : "INVALID");
    region_dump(r1, "");
    expected[EXPECT_R1_EMPTY] = FALSE;
    expected[EXPECT_R2_EMPTY] = TRUE;
    expected[EXPECT_EQUAL] = FALSE;
    expected[EXPECT_SECT] = FALSE;
    expected[EXPECT_CONT] = TRUE;
    test(r1, r2, expected);
    printf("\n");

    rect_set(r, 500, 500, 600, 600);
    region_add(r2, r);
    printf("dump r2 [%s]\n", region_is_valid(r2) ? "VALID" : "INVALID");
    region_dump(r2, "");
    expected[EXPECT_R1_EMPTY] = FALSE;
    expected[EXPECT_R2_EMPTY] = FALSE;
    expected[EXPECT_EQUAL] = FALSE;
    expected[EXPECT_SECT] = FALSE;
    expected[EXPECT_CONT] = FALSE;
    test(r1, r2, expected);
    printf("\n");

    region_clear(r2);

    rect_set(r, 100, 100, 200, 200);
    region_add(r2, r);
    rect_set(r, 300, 300, 400, 400);
    region_add(r2, r);
    printf("dump r2 [%s]\n", region_is_valid(r2) ? "VALID" : "INVALID");
    region_dump(r2, "");
    expected[EXPECT_R1_EMPTY] = FALSE;
    expected[EXPECT_R2_EMPTY] = FALSE;
    expected[EXPECT_EQUAL] = TRUE;
    expected[EXPECT_SECT] = TRUE;
    expected[EXPECT_CONT] = TRUE;
    test(r1, r2, expected);
    printf("\n");

    region_clear(r2);

    rect_set(r, 100, 100, 200, 200);
    region_add(r2, r);
    printf("dump r2 [%s]\n", region_is_valid(r2) ? "VALID" : "INVALID");
    region_dump(r2, "");
    expected[EXPECT_R1_EMPTY] = FALSE;
    expected[EXPECT_R2_EMPTY] = FALSE;
    expected[EXPECT_EQUAL] = FALSE;
    expected[EXPECT_SECT] = TRUE;
    expected[EXPECT_CONT] = TRUE;
    test(r1, r2, expected);
    printf("\n");

    region_clear(r2);

    rect_set(r, -2000, -2000, -1000, -1000);
    region_add(r2, r);
    printf("dump r2 [%s]\n", region_is_valid(r2) ? "VALID" : "INVALID");
    region_dump(r2, "");
    expected[EXPECT_R1_EMPTY] = FALSE;
    expected[EXPECT_R2_EMPTY] = FALSE;
    expected[EXPECT_EQUAL] = FALSE;
    expected[EXPECT_SECT] = FALSE;
    expected[EXPECT_CONT] = FALSE;
    test(r1, r2, expected);
    printf("\n");

    region_clear(r2);

    rect_set(r, -2000, -2000, 1000, 1000);
    region_add(r2, r);
    printf("dump r2 [%s]\n", region_is_valid(r2) ? "VALID" : "INVALID");
    region_dump(r2, "");
    expected[EXPECT_R1_EMPTY] = FALSE;
    expected[EXPECT_R2_EMPTY] = FALSE;
    expected[EXPECT_EQUAL] = FALSE;
    expected[EXPECT_SECT] = TRUE;
    expected[EXPECT_CONT] = FALSE;
    test(r1, r2, expected);
    printf("\n");

    region_clear(r2);

    rect_set(r, 150, 150, 175, 175);
    region_add(r2, r);
    printf("dump r2 [%s]\n", region_is_valid(r2) ? "VALID" : "INVALID");
    region_dump(r2, "");
    expected[EXPECT_R1_EMPTY] = FALSE;
    expected[EXPECT_R2_EMPTY] = FALSE;
    expected[EXPECT_EQUAL] = FALSE;
    expected[EXPECT_SECT] = TRUE;
    expected[EXPECT_CONT] = TRUE;
    test(r1, r2, expected);
    printf("\n");

    region_clear(r2);

    rect_set(r, 150, 150, 350, 350);
    region_add(r2, r);
    printf("dump r2 [%s]\n", region_is_valid(r2) ? "VALID" : "INVALID");
    region_dump(r2, "");
    expected[EXPECT_R1_EMPTY] = FALSE;
    expected[EXPECT_R2_EMPTY] = FALSE;
    expected[EXPECT_EQUAL] = FALSE;
    expected[EXPECT_SECT] = TRUE;
    expected[EXPECT_CONT] = FALSE;
    test(r1, r2, expected);
    printf("\n");

    region_and(r2, r1);
    printf("dump r2 and r1 [%s]\n", region_is_valid(r2) ? "VALID" : "INVALID");
    region_dump(r2, "");
    expected[EXPECT_R1_EMPTY] = FALSE;
    expected[EXPECT_R2_EMPTY] = FALSE;
    expected[EXPECT_EQUAL] = FALSE;
    expected[EXPECT_SECT] = TRUE;
    expected[EXPECT_CONT] = FALSE;
    test(r2, r1, expected);
    printf("\n");


    region_clone(r3, r1);
    printf("dump r3 clone rgn [%s]\n", region_is_valid(r3) ? "VALID" : "INVALID");
    region_dump(r3, "");
    expected[EXPECT_R1_EMPTY] = FALSE;
    expected[EXPECT_R2_EMPTY] = FALSE;
    expected[EXPECT_EQUAL] = TRUE;
    expected[EXPECT_SECT] = TRUE;
    expected[EXPECT_CONT] = TRUE;
    test(r1, r3, expected);
    printf("\n");

    j = 0;
    for (i = 0; i < 1000000; i++) {
        int res1, res2, test;
        int tests[] = {
            REGION_TEST_LEFT_EXCLUSIVE,
            REGION_TEST_RIGHT_EXCLUSIVE,
            REGION_TEST_SHARED,
            REGION_TEST_LEFT_EXCLUSIVE | REGION_TEST_RIGHT_EXCLUSIVE,
            REGION_TEST_LEFT_EXCLUSIVE | REGION_TEST_SHARED,
            REGION_TEST_RIGHT_EXCLUSIVE | REGION_TEST_SHARED,
            REGION_TEST_LEFT_EXCLUSIVE | REGION_TEST_RIGHT_EXCLUSIVE | REGION_TEST_SHARED
        };

        random_region(r1);
        random_region(r2);

        for (test = 0; test < 7; test++) {
            res1 = region_test(r1, r2, tests[test]);
            res2 = slow_region_test(r1, r2, tests[test]);
            if (res1 != res2) {
                printf ("Error in region_test %d, got %d, expected %d, query=%d\n",
                        j, res1, res2, tests[test]);
                printf ("r1:\n");
                region_dump(r1, "");
                printf ("r2:\n");
                region_dump(r2, "");
            }
            j++;
        }
    }

    region_destroy(r3);
    region_destroy(r1);
    region_destroy(r2);

    return 0;
}
