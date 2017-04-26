#include <stdlib.h>

#include "bimage.h"

#define IMAGE_OP(name) \
BIMAGE_STATUS \
bimage##name(bimage* a, bimage* b) \
{ \
    bpixel p, q; \
    bimageIterAll(a, x, y){ \
        if (bimageGetPixelUnsafe(a, x, y, &p) == BIMAGE_ERR  || \
            bimageGetPixel(b, x, y, &q) == BIMAGE_ERR || \
            bpixel##name(&p, &q) == BIMAGE_ERR){ \
            break; \
        } \
        bimageSetPixelUnsafe(a, x, y, &p); \
    } \
    return BIMAGE_OK; \
}

IMAGE_OP(Add);
IMAGE_OP(Sub);
IMAGE_OP(Mul);
IMAGE_OP(Div);

bimage*
bimageColor(bimage** dst, bimage* im, BIMAGE_CHANNEL c)
{

    if (c < 3){
        return NULL;
    }

    BIMAGE_TYPE t;
    BIMAGE_DEPTH depth = bimageTypeDepth(im->type);
    bpixel px;

    if (bimageMakeType(c, depth, &t) == BIMAGE_ERR){
        return NULL;
    }

    bimage* im2 = BIMAGE_CREATE_DEST(dst, im->width, im->height, t);
    if (im2 == NULL){
        return NULL;
    }

    bimageIterAll(im2, x, y){
        bimageGetPixelUnsafe(im, x, y, &px);
        bimageSetPixel(im2, x, y, &px);
    }

    BIMAGE_RETURN_DEST(dst, im2);
}

bimage*
bimageGrayscale(bimage** dst, bimage* im, BIMAGE_CHANNEL chan)
{
    BIMAGE_TYPE t;
    bpixel p, px;
    BIMAGE_DEPTH depth = bimageTypeDepth(im->type);

    if (bimageMakeType(chan, depth, &t) == BIMAGE_ERR){
        return NULL;
    }

    bimage *im2 = BIMAGE_CREATE_DEST(dst, im->width, im->height, t);
    if (im2 == NULL){
        return NULL;
    }

    p.depth = depth;
    p.data[3] = bimageTypeMax(im->type);
    bimageIterAll(im, x, y){
        bimageGetPixelUnsafe(im, x, y, &px);

        p.data[0] = p.data[1] = p.data[2] = (px.data[0] * 0.2126) + (px.data[1] * 0.7152) + (px.data[2] * 0.0722) * (px.data[3] / (float)bimageTypeMax(im->type));

        bimageSetPixel(im2, x, y, &p);
    }

    BIMAGE_RETURN_DEST(dst, im2);
}

// Convolution filter
bimage*
bimageFilter(bimage** dst, bimage* im, float* K, int Ks, float divisor, float offset)
{
    bimage *oi = BIMAGE_CREATE_DEST(dst, im->width, im->height, im->type);
    if (oi == NULL){
        return NULL;
    }

    Ks = Ks/2;

    int channels = bimageTypeChannels(im->type);
    int32_t ix, iy;
    int kx, ky, l;
    bpixel p, px;
    px.depth = bimageTypeDepth(im->type);
    px.data[3] = bimageTypeMax(im->type);

    // Ignore alpha channel
    if (channels > BIMAGE_RGB){
        channels = BIMAGE_RGB;
    }

    // Divisor can never be zero
    if (divisor == 0.0){
        divisor = 1.0;
    }

    for(ix = 0; ix < im->width; ix++){
        for(iy = 0; iy < im->height; iy++){
            px.data[0] = px.data[1] = px.data[2] = 0.0;
            for(kx = -Ks; kx <= Ks; kx++){
                for(ky = -Ks; ky <= Ks; ky++){
                    bimageGetPixel(im, ix+kx, iy+ky, &p);
                    for (l = 0; l < channels; l++){
                        px.data[l] += (K[(kx+Ks) + (ky+Ks)*(2*Ks+1)]/divisor) * p.data[l] + offset;
                    }
                }
            }

            for(l = 0; l < channels; l++){
                px.data[l] = px.data[l] > (float)bimageTypeMax(im->type) ? bimageTypeMax(im->type) : px.data[l] < 0 ? 0 : px.data[l];
            }

            bimageSetPixel(oi, ix, iy, &px);
        }
    }

    BIMAGE_RETURN_DEST(dst, oi);
}

bimage*
bimageInvert(bimage** dst, bimage* src)
{
    bpixel px;
    int i;
    bimage *im2 = BIMAGE_CREATE_DEST(dst, src->width, src->height, src->type);
    if (im2 == NULL){
        return NULL;
    }

    BIMAGE_CHANNEL ch = bimageTypeChannels(src->type);
    float mx = bimageTypeMax(src->type);

    bimageIterAll(im2, x, y){
        bimageGetPixelUnsafe(im2, x, y, &px);
        for(i = 0; i < ch; i++){
            px.data[i] = mx - px.data[i];
        }
        bimageSetPixelUnsafe(im2, x, y, &px);
    }

    BIMAGE_RETURN_DEST(dst, im2);
}

static float sobel_x[9] = {
    1, 0, -1,
    2, 0, -2,
    1, 0, -1
};

bimage*
bimageSobelX(bimage** dst, bimage* src)
{
    return bimageFilter(dst, src, sobel_x, 3, 1, 0);
}

static float sobel_y[9] = {
    1, 2, 1,
    0, 0, 0,
    -1, -2, -1
};

bimage*
bimageSobelY(bimage** dst, bimage* src)
{
    return bimageFilter(dst, src, sobel_y, 3, 1, 0);
}

bimage*
bimageSobel(bimage**dst, bimage* src){
    bimage* src2 = bimageSobelX(dst, src);
    if (!src2){
        return NULL;
    }

    bimage* tmp = bimageSobelY(NULL, src2);
    if (!tmp){
        if (!dst || !*dst){
            bimageRelease(src2);
        }
        return NULL;
    }

    bimageAdd(src2, tmp);
    return src2;
}

static float prewitt_x[9] = {
    1, 0, -1,
    1, 0, -1,
    1, 0, -1
};

bimage*
bimagePrewittX(bimage** dst, bimage* src)
{
    return bimageFilter(dst, src, sobel_x, 3, 1, 0);
}

static float prewitt_y[9] = {
    1, 1, 1,
    0, 0, 0,
    -1, -1, -1
};

bimage*
bimagePrewittY(bimage** dst, bimage* src)
{
    return bimageFilter(dst, src, sobel_y, 3, 1, 0);
}

bimage*
bimagePrewitt(bimage**dst, bimage* src){
    bimage* src2 = bimagePrewittX(dst, src);
    if (!src2){
        return NULL;
    }

    bimage* tmp = bimagePrewittY(NULL, src2);
    if (!tmp){
        if (!dst || !*dst){
            bimageRelease(src2);
        }
        return NULL;
    }

    bimageAdd(src2, tmp);

    return src2;
}

static float outline[9] = {
    -1, -1, -1
    -1,  8, -1,
    -1, -1, -1
};

bimage*
bimageOutline(bimage** dst, bimage* im)
{
    return bimageFilter(dst, im, outline, 3, 1, 0);
}


static float sharpen[9] = {
     0, -1,  0,
    -1,  5, -1,
     0, -1,  0
};

bimage*
bimageSharpen(bimage** dst, bimage* im)
{
    return bimageFilter(dst, im, sharpen, 3, 1, 0);
}

static float blur[9] = {
    1, 1, 1,
    1, 1, 1,
    1, 1, 1
};

bimage*
bimageBlur(bimage** dst, bimage* im)
{
    return bimageFilter(dst, im, blur, 3, 9, 0);
}

static float gaussian_blur[25] = {
    1,  4,  7,  4,  1,
    4, 16, 26, 16,  4,
    7, 26, 41, 26,  7,
    4, 16, 26, 16,  4,
    1,  4,  7,  4,  1
};

bimage*
bimageGaussianBlur(bimage** dst, bimage* im)
{
    return bimageFilter(dst, im, gaussian_blur, 5, 273, 0);
}
