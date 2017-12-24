#include <stdlib.h>
#include <math.h>

#include "bimage.h"

#define IMAGE_OP(name) \
BIMAGE_STATUS \
bimage##name(bimage* a, bimage* b) \
{ \
    bimagePixel p, q; \
    bimageIterAll(a, x, y){ \
        if (bimageGetPixelUnsafe(a, x, y, &p) == BIMAGE_ERR  || \
            bimageGetPixel(b, x, y, &q) == BIMAGE_ERR || \
            bimagePixel##name(&p, q) == BIMAGE_ERR){ \
            break; \
        } \
        bimagePixelClamp(&p); \
        bimageSetPixelUnsafe(a, x, y, p); \
    } \
    return BIMAGE_OK; \
}

IMAGE_OP(Add);
IMAGE_OP(Sub);
IMAGE_OP(Mul);
IMAGE_OP(Div);

static void bimageColorFn(uint32_t x, uint32_t y, bimagePixel *px, void *userdata){
    bimage* im2 = userdata;
    bimageSetPixel(im2, x, y, *px);
}


bimage*
bimageColor(bimage* dst, bimage* im, BIMAGE_CHANNEL c)
{

    if (c < 3){
        return NULL;
    }

    BIMAGE_DEPTH depth = bimageTypeDepth(im->type);

    bimage* im2 = BIMAGE_CREATE_DEST(dst, im->width, im->height, depth | c);
    if (im2 == NULL){
        return NULL;
    }

#ifndef BIMAGE_NO_PTHREAD
    bimageParallel(im, bimageColorFn, BIMAGE_NUM_CPU, im2);
#else
    bimagePixel px;
    bimageIterAll(im2, x, y){
        bimageGetPixelUnsafe(im, x, y, &px);
        bimageSetPixel(im2, x, y, px);
    }
#endif

    return im2;
}

static void bimageGrayscaleFn(uint32_t x, uint32_t y, bimagePixel *px, void *userdata){
    bimage *im2 = userdata;
    float mx = (float)bimageTypeMax(im2->type);
    bimagePixel p;
    p.depth = bimageTypeDepth(im2->type);
    p.data.f[3] = bimageTypeMax(im2->type);
    p.data.f[0] = p.data.f[1] = p.data.f[2] = (px->data.f[0] * 0.2126) + (px->data.f[1] * 0.7152) + (px->data.f[2] * 0.0722) * (px->data.f[3] / mx);
    bimageSetPixelUnsafe(im2, x, y, p);

}

bimage*
bimageGrayscale(bimage* dst, bimage* im, BIMAGE_CHANNEL chan)
{

    BIMAGE_DEPTH depth = bimageTypeDepth(im->type);

    bimage *im2 = BIMAGE_CREATE_DEST(dst, im->width, im->height, depth | chan);
    if (im2 == NULL){
        return NULL;
    }

#ifndef BIMAGE_NO_PTHREAD
    bimageParallel(im, bimageGrayscaleFn, BIMAGE_NUM_CPU, im2);
#else
    bimagePixel p, px;
    bimageIterAll(im, x, y){
        bimageGetPixelUnsafe(im, x, y, &px);
        bimageGrayscaleFn(x, y, &px, im2);
    }
#endif

    return im2;
}

bimage*
bimageInvert(bimage* dst, bimage* src)
{
    bimagePixel px;
    int i;
    bimage *im2 = BIMAGE_CREATE_DEST(dst, src->width, src->height, src->type);
    if (im2 == NULL){
        return NULL;
    }

    BIMAGE_CHANNEL ch = bimageTypeChannels(src->type);
    float mx = (float)bimageTypeMax(src->type);

    bimageIterAll(im2, x, y){
        bimageGetPixelUnsafe(im2, x, y, &px);
        for(i = 0; i < ch % 5; i++){
            px.data.f[i] = mx - px.data.f[i];
        }
        bimageSetPixelUnsafe(im2, x, y, px);
    }

    return im2;
}

bimage*
bimageRotate(bimage* dst, bimage* im, float deg)
{
    bimage* im2 = BIMAGE_CREATE_DEST(dst, im->width, im->height, im->type);
    if (!im2){
        return NULL;
    }

    float midX, midY;
    float dx, dy;
    int32_t rotX, rotY;

    midX = im->width / 2.0f;
    midY = im->height / 2.0f;

    float angle = 2 * M_PI * deg / 360.0f;

    bimagePixel px;
    bimageIterAll(im2, i, j){
        dx = i + 0.5 - midX;
        dy = j + 0.5 - midY;

        rotX = (uint32_t)(midX + dx * cos(angle) - dy * sin(angle));
        rotY = (uint32_t)(midY + dx * sin(angle) + dy * cos(angle));
        if (rotX >= 0 && rotY >= 0){
            if (bimageGetPixel(im, rotX, rotY, &px) == BIMAGE_OK){
                bimageSetPixel(im2, i, j, px);
            }
        }
    }

    return im2;
}
