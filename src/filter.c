#include <stdlib.h>

#include "bimage.h"

bimage*
bimageGrayscale(bimage** dst, bimage *im)
{
    BIMAGE_TYPE t;
    bpixel p, px;
    int depth = bimageTypeDepth(im->type);

    if (bimageMakeType(1, depth, &t) == BIMAGE_ERR){
        return NULL;
    }

    bimage *im2 = BIMAGE_CREATE_DEST(dst, im->width, im->height, t);
    if (!dst){
        return NULL;
    }

    p.depth = depth;
    bimageIterAll(im, x, y){
        bimageGetPixelUnsafe(im, x, y, &px);

        p.data[0] = p.data[1] = p.data[2] = (px.data[0] * 0.2126) + (px.data[1] * 0.7152) + (px.data[2] * 0.0722) * (px.data[3] / (float)bimageTypeMax(im->type));

        bimageSetPixel(im2, x, y, p);
    }

    *dst = im2;
    return *dst;
}

// Convolution filter
bimage*
bimageFilter(bimage** dst, bimage* im, float* K, int Ks, float divisor, float offset)
{
    bimage *oi = BIMAGE_CREATE_DEST(dst, im->width, im->height, im->type);
    if (oi == NULL){
        return NULL;
    }

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

            bimageSetPixel(oi, ix, iy, px);
        }
    }

    *dst = oi;
    return *dst;
}
