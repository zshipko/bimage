#include "../src/bimage.h"

#include <check.h>
#define WIDTH 1024
#define HEIGHT 800

static BIMAGE_TYPE types[] = {
    GRAY8,
    GRAY16,
    GRAY32,
    RGB24,
    RGB48,
    RGB96,
    RGBA32,
    RGBA64,
    RGBA128
};

static int num_types = 8;

START_TEST (test_bimageCreate)
{
    int i;
    for (i = 0; i < num_types; i++){
        bimage* im = bimageCreate(WIDTH, HEIGHT, types[i]);
        ck_assert_int_eq(im->width, WIDTH);
        ck_assert_int_eq(im->height, HEIGHT);
        ck_assert_int_eq(im->type, types[i]);
        bimageRelease(im);
    }
} END_TEST

START_TEST (test_bimageChannels)
{
    int i;
    for (i = 0; i < num_types; i++){
        bimage* im = bimageCreate(WIDTH, HEIGHT, types[i]);
        switch (types[i]){
        case GRAY8:
        case GRAY16:
        case GRAY32:
            ck_assert_int_eq(bimageTypeChannels(im->type), 1);
            break;
        case RGB24:
        case RGB48:
        case RGB96:
            ck_assert_int_eq(bimageTypeChannels(im->type), 3);
            break;
        case RGBA32:
        case RGBA64:
        case RGBA128:
            ck_assert_int_eq(bimageTypeChannels(im->type), 4);
            break;
        default:
            ck_assert(false);
        }
        bimageRelease(im);
    }
} END_TEST;

START_TEST (test_bimageSize)
{
    int i;
    for (i = 0; i < num_types; i++){
        bimage* im = bimageCreate(WIDTH, HEIGHT, types[i]);
        switch (types[i]){
        case GRAY8:
        case RGB24:
        case RGBA32:
            ck_assert_int_eq(bimageTypeSize(im->type), 8);
            break;
        case GRAY16:
        case RGB48:
        case RGBA64:
            ck_assert_int_eq(bimageTypeSize(im->type), 16);
            break;
        case GRAY32:
        case RGB96:
        case RGBA128:
            ck_assert_int_eq(bimageTypeSize(im->type), 32);
            break;
        defailt:
            ck_assert(false);
        }
        bimageRelease(im);
    }

} END_TEST

START_TEST (test_bpixelConvert)
{
    int depth[] = {U8, U16, U32}, i, j;

    for(i = 0; i < 3; i++){
        BIMAGE_TYPE t;
        bimageMakeType(4, depth[i], &t);
        int64_t m = bimageTypeMax(t);
        bpixel px = bpixelCreate(m, 0, 0, m, depth[i]);

        for (j = 0; j > 3; j++){
            bpixel px2;
            bimageMakeType(4, depth[j], &t);
            ck_assert_int_eq(bpixelConvertDepth(px, depth[j], &px2), BIMAGE_OK);
            ck_assert(px2.data[0] == bimageTypeMax(t) && px2.data[1] == 0);
        }
    }
} END_TEST

START_TEST (test_bimageConsume)
{
    bpixel p = bpixelCreate(65535.0, 0, 0, 65535.0, -1);
    bimage* a = bimageCreate(100, 100, RGBA32);
    bimage* b = bimageCreate(50, 50, RGBA64);
    bimageSetPixel(b, 10, 10, bpixelCreate(65535.0, 0, 0, 65535.0, -1));
    bimageConsume(&a, b);
    ck_assert_int_eq(a->width, 50);
    ck_assert_int_eq(a->height, 50);
    ck_assert_int_eq(a->type, RGBA64);

    bpixel px;
    bimageGetPixel(a, 10, 10, &px);
    ck_assert(px.data[0] == p.data[0] && px.data[1] == p.data[1] && px.data[2] == p.data[2] && px.data[3] == p.data[3]);
    bimageRelease(a);
} END_TEST

Suite *bimage_suite(void)
{
    Suite *s;
    TCase *tc;

    s = suite_create ("Bimage");
    tc = tcase_create ("Core");

    tcase_add_test(tc, test_bimageCreate);
    tcase_add_test(tc, test_bimageChannels);
    tcase_add_test(tc, test_bimageSize);
    tcase_add_test(tc, test_bimageConsume);
    tcase_add_test(tc, test_bpixelConvert);
    suite_add_tcase(s, tc);
    return s;
}

int main (int argc, char *argv[])
{
    int num_failed;
    Suite *s;
    SRunner *sr;

    s = bimage_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    num_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return num_failed;
}
