#include "bench.h"
#include "../src/bimage.h"

#include <check.h>
#define WIDTH 1024
#define HEIGHT 800

static BIMAGE_TYPE types[] = {
    BIMAGE_GRAY | BIMAGE_U8,
    BIMAGE_GRAY | BIMAGE_U16,
    BIMAGE_GRAY | BIMAGE_U32,
    BIMAGE_GRAY | BIMAGE_F32,
    BIMAGE_RGB | BIMAGE_U8,
    BIMAGE_RGB | BIMAGE_U16,
    BIMAGE_RGB | BIMAGE_U32,
    BIMAGE_RGB | BIMAGE_F32,
    BIMAGE_RGBA | BIMAGE_U8,
    BIMAGE_RGBA | BIMAGE_U16,
    BIMAGE_RGBA | BIMAGE_U32,
    BIMAGE_RGBA | BIMAGE_F32,
};

static int num_types = 8;

bimage* randomImage(uint32_t w, uint32_t h, BIMAGE_TYPE t)
{
    bimage* im = bimageCreate(w, h, t);
    if (!im){
        return NULL;
    }

    bimageIterAll(im, x, y){
        bimageSetPixel(im, x, y, bimagePixelRandom(bimageTypeDepth(t)));
    }

    if (bimageTypeDepth(t) == BIMAGE_U8) {
        bimageSave(im, "random.png");
    }

    return im;
}

START_TEST (test_bimageCreate)
{
    int i;
    for (i = 0; i < num_types; i++){
        BENCH_START(create);
        bimage* im = bimageCreate(WIDTH, HEIGHT, types[i]);
        BENCH_STOP(create);
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
        bimage* im = randomImage(WIDTH, HEIGHT, types[i]);
        switch (bimageTypeChannels(types[i])){
        case BIMAGE_GRAY:
            ck_assert_int_eq(bimageTypeChannels(im->type), 1);
            break;
        case BIMAGE_RGB:
            ck_assert_int_eq(bimageTypeChannels(im->type), 3);
            break;
        case BIMAGE_RGBA:
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
        bimage* im = randomImage(WIDTH, HEIGHT, types[i]);
        switch (bimageTypeDepth(types[i])){
        case BIMAGE_U8:
            ck_assert_int_eq(bimageTypeDepth(im->type), BIMAGE_U8);
            break;
        case BIMAGE_U16:
            ck_assert_int_eq(bimageTypeDepth(im->type), BIMAGE_U16);
            break;
        case BIMAGE_U32:
            ck_assert_int_eq(bimageTypeDepth(im->type), BIMAGE_U32);
            break;
        case BIMAGE_F32:
            ck_assert_int_eq(bimageTypeDepth(im->type), BIMAGE_F32);
            break;
        defailt:
            ck_assert(false);
        }
        bimageRelease(im);
    }

} END_TEST

START_TEST (test_bimagePixelConvert)
{
    BIMAGE_DEPTH depth[] = {BIMAGE_U8, BIMAGE_U16, BIMAGE_U32, BIMAGE_F32}, i, j;

    for(i = 0; i < 4; i++){
        BIMAGE_TYPE t = depth[i] | 4;
        int64_t m = bimageTypeMax(t);
        bimagePixel px = bimagePixelCreate(m, 0, 0, m, depth[i]);

        for (j = 0; j > 3; j++){
            bimagePixel px2;
            t = depth[j] | 4;
            ck_assert_int_eq(bimagePixelConvertDepth(&px, px2, depth[j]), BIMAGE_OK);
            ck_assert(px2.data.f[0] == bimageTypeMax(t) && px2.data.f[1] == 0);
        }
    }
} END_TEST

START_TEST (test_bimageConsume)
{
    bimagePixel p = bimagePixelCreate(65535.0, 0, 0, 65535.0, BIMAGE_U16);
    bimage* a = randomImage(100, 100, BIMAGE_U8 | 4);
    bimage* b = randomImage(50, 50, BIMAGE_U16 | 4);
    bimageSetPixel(b, 10, 10, p);
    bimageConsume(&a, b);
    ck_assert_int_eq(a->width, 50);
    ck_assert_int_eq(a->height, 50);
    ck_assert_int_eq(bimageHash(a), bimageHash(b));

    char as[9], bs[9];
    bimageHashString(as, bimageHash(a));
    bimageHashString(bs, bimageHash(b));
    ck_assert(strncmp(as, bs, 8) == 0);

    bimagePixel px;
    bimageGetPixel(a, 10, 10, &px);
    ck_assert(px.data.f[0] == p.data.f[0] && px.data.f[1] == p.data.f[1] && px.data.f[2] == p.data.f[2] && px.data.f[3] == p.data.f[3]);
    bimageRelease(a);
} END_TEST

START_TEST (test_bimageAdd)
{
    bimage* im = randomImage(100, 100, BIMAGE_U8 | 3);
    bimage* im2 = randomImage(100, 100, BIMAGE_U8 | 3);

    bimagePixel c = bimagePixelCreate(255, 0, 0, 255, BIMAGE_U8), d;
    bimageSetPixelUnsafe(im2, 50, 50, c);

    BENCH_START(add);
    bimageAdd(im, im2);
    BENCH_STOP(add);

    bimageGetPixelUnsafe(im2, 50, 50, &d);
    ck_assert(d.data.f[0] == c.data.f[0] && d.data.f[1] == c.data.f[1] && d.data.f[2] == c.data.f[2]);
    bimageRelease(im);
    bimageRelease(im2);
} END_TEST;

START_TEST (test_bimageResizeHash)
{
    int i;
    for (i = 0; i < num_types; i++){
        bimage* im = randomImage(WIDTH, HEIGHT, types[i]);
        bimage* im2 = randomImage(111, 222, types[i]);
        bimageResize(im2, im, 111, 222);
        ck_assert_int_eq(im2->width, 111);
        ck_assert_int_eq(im2->height, 222);
        ck_assert_int_eq(bimageHash(im), bimageHash(im2));
        bimageRelease(im);
        bimageRelease(im2);
    }

} END_TEST;

START_TEST (test_bimageCopyTo)
{
    int i;
    for (i = 0; i < num_types; i++){
        bimage* im = randomImage(WIDTH, HEIGHT, types[i]);
        bimage* im2 = randomImage(WIDTH/2, HEIGHT/2, types[i]);

        BENCH_START(copyTo);
        bimageCopyTo(im, im2, 10, 10);
        BENCH_STOP(copyTo);

        bimagePixel a, b;
        bimageGetPixel(im, 10, 10, &a);
        bimageGetPixel(im2, 0, 0, &b);

        ck_assert(a.data.f[0] == b.data.f[0] && a.data.f[1] == b.data.f[1] && a.data.f[2] == b.data.f[2]);
        bimageGetPixel(im, im2->width+9, im2->height+9, &a);
        bimageGetPixel(im2, im2->width-1, im2->height-1, &b);
        ck_assert(a.data.f[0] == b.data.f[0] && a.data.f[1] == b.data.f[1] && a.data.f[2] == b.data.f[2]);
        bimageRelease(im);
        bimageRelease(im2);
    }
} END_TEST;

START_TEST (test_bimageGrayscale)
{
    int i;
    for (i = 0; i < num_types; i++){
        bimage* im = randomImage(WIDTH, HEIGHT, types[i]);

        BENCH_START(grayscale);
        bimage* im2 = bimageGrayscale(NULL, im, 1);
        BENCH_STOP(grayscale);

        bimagePixel px;
        bimageGetPixel(im2, 0, 0, &px);
        ck_assert(px.data.f[0] == px.data.f[1] && px.data.f[1] == px.data.f[2]);
        bimageGetPixel(im2, WIDTH-1, HEIGHT-1, &px);
        ck_assert(px.data.f[0] == px.data.f[1] && px.data.f[1] == px.data.f[2]);

        if (types[i] == BIMAGE_U8 | BIMAGE_RGB){
            bimageSave(im2, "test.jpg");
        }

        bimageRelease(im2);
        bimageRelease(im);
    }
} END_TEST;

bool parallel_fn(uint32_t x, uint32_t y, bimagePixel *px, void *userdata){
    px->data.f[0] = px->data.f[1] = px->data.f[2] = px->data.f[3] = 1.0;
    return true;
}

#ifndef BIMAGE_NO_PTHREAD
START_TEST (test_bimageParallel)
{
    bimage* im = randomImage(WIDTH, HEIGHT, BIMAGE_F32);
    ck_assert(bimageParallel(im, parallel_fn, 8, NULL) == BIMAGE_OK);

    for(size_t i = 0; i < im->width * im->height * bimageTypeChannels(im->type); i++){
        BENCH_START(parallel);
        ck_assert(bimageAt(im, i, float) == 1.0);
        BENCH_STOP(parallel);
    }
    bimageRelease(im);
} END_TEST;
#endif

START_TEST (test_bimageDisk)
{
    BENCH_START(disk0);
    bimage *im = bimageCreateOnDisk ("/tmp/bimage", 500, 500, BIMAGE_F32|3);
    BENCH_STOP(disk0);
    bimagePixel px = bimagePixelRandom(BIMAGE_F32), px2;
    bimageSetPixelUnsafe(im, 25, 25, px);
    bimageRelease(im);

    BENCH_START(disk1);
    im = bimageCreateOnDisk ("/tmp/bimage", 0, 0, 0);
    BENCH_STOP(disk1);
    ck_assert((im->width == 500 && im->height == 500 && (im->type == (BIMAGE_F32|3))));

    bimageGetPixelUnsafe(im, 25, 25, &px2);
    ck_assert((px2.data.f[0] == px.data.f[0] && px2.data.f[1] == px.data.f[1] && px2.data.f[2] == px.data.f[2]));

    bimageRelease(im);
    unlink("/tmp/bimage");
} END_TEST;

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
    tcase_add_test(tc, test_bimagePixelConvert);
    tcase_add_test(tc, test_bimageAdd);
    tcase_add_test(tc, test_bimageResizeHash);
    tcase_add_test(tc, test_bimageCopyTo);
    tcase_add_test(tc, test_bimageGrayscale);
#ifndef BIMAGE_NO_PTHREAD
    tcase_add_test(tc, test_bimageParallel);
#endif
    tcase_add_test(tc, test_bimageDisk);
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
