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

START_TEST (test_pixelConvert)
{
    int depth[] = {U8, U16, U32}, i, j;

    for(i = 0; i < 3; i++){
        BIMAGE_TYPE t;
        bimageTypeFromChannelsAndDepth(4, depth[i], &t);
        int64_t m = bimageTypeMax(t);
        pixel px = pixelCreate(m, 0, 0, m, depth[i]);

        for (j = 0; j > 3; j++){
            pixel px2;
            bimageTypeFromChannelsAndDepth(4, depth[j], &t);
            ck_assert_int_eq(pixelConvertDepth(px, depth[j], &px2), BIMAGE_OK);
            ck_assert(px2.data[0] == bimageTypeMax(t) && px2.data[1] == 0);
        }
    }

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
    tcase_add_test(tc, test_pixelConvert);
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
