#include "bimage.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>


BIMAGE_STATUS
bhistogramInit(bhistogram* hist)
{
    if (!hist){
        return BIMAGE_ERR;
    }

    bzero(&hist->bucket, sizeof(float) * 256);
    hist->total = 0L;

    return BIMAGE_OK;
}

BIMAGE_STATUS
bimageHistogram(bimage *im, bhistogram h[], BIMAGE_CHANNEL ch)
{
    int i;

    for(i = 0; i < ch; i++){
        if (bhistogramInit(&h[i]) == BIMAGE_ERR){
            return BIMAGE_ERR;
        }
    }

    bpixel px, pc;
    bimageIterAll(im, x, y){
        bimageGetPixelUnsafe(im, x, y, &px);
        bpixelConvertDepth(&pc, px, BIMAGE_U8);

        for (i = 0; i < ch; i++){
            h[i].bucket[(int)pc.data[i]] += 1;
            h[i].total += 1L;
        }
    }

    return BIMAGE_OK;
}

int
bhistogramMax(bhistogram h)
{
    double m = 0;
    int i, c = 0;

    for(i = 0; i < 256; i++){
        if (h.bucket[i] > m){
            m = h.bucket[i];
            c = i;
        }
    }

    return c;
}

int
bhistogramMin(bhistogram h)
{
    double m = (double)h.bucket[bhistogramMax(h)];
    int i, c = 0;

    for(i = 0; i < 256; i++){
        if (h.bucket[i] < m && h.bucket[i] > 0){
            m = h.bucket[i];
            c = i;
        }
    }

    return c;
}

bimage*
bhistogramImage(bhistogram h)
{
    bimage *im = bimageCreate(256, 256, BIMAGE_U8 | BIMAGE_GRAY);
    if (!im){
        return NULL;
    }

    double mx = h.bucket[bhistogramMax(h)];
    double mn = h.bucket[bhistogramMin(h)];

    int i, n;
    bpixel px = bpixelCreate(255, 255, 255, 255, BIMAGE_U8);
    for(i = 0; i < 256; i++){
        double x = (h.bucket[i] - mn) * 255.0 / (mx - mn);
        for(n = 1; n < x; n++){
            bimageSetPixel(im, i, im->height-n, px);
        }
    }

    return im;
}
