#include "bimage.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>


BIMAGE_STATUS
bimageHistogramInit(bimageHistogram* hist)
{
    if (!hist){
        return BIMAGE_ERR;
    }

    bzero(&hist->bucket, sizeof(float) * 256);
    hist->total = 0L;

    return BIMAGE_OK;
}

BIMAGE_STATUS
bimageGetHistogram(bimage *im, bimageHistogram h[], BIMAGE_CHANNEL ch)
{
    int i;

    for(i = 0; i < ch; i++){
        if (bimageHistogramInit(&h[i]) == BIMAGE_ERR){
            return BIMAGE_ERR;
        }
    }

    bimagePixel px, pc;
    bimageIterAll(im, x, y){
        bimageGetPixelUnsafe(im, x, y, &px);
        bimagePixelConvertDepth(&pc, px, BIMAGE_U8);

        for (i = 0; i < ch; i++){
            h[i].bucket[(int)pc.data.f[i]] += 1;
            h[i].total += 1L;
        }
    }

    return BIMAGE_OK;
}

int
bimageHistogramMax(bimageHistogram h)
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
bimageHistogramMin(bimageHistogram h)
{
    double m = (double)h.bucket[bimageHistogramMax(h)];
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
bimageHistogramImage(bimageHistogram h)
{
    bimage *im = bimageCreate(256, 256, BIMAGE_U8 | BIMAGE_GRAY);
    if (!im){
        return NULL;
    }

    double mx = h.bucket[bimageHistogramMax(h)];
    double mn = h.bucket[bimageHistogramMin(h)];

    int i, n;
    bimagePixel px = bimagePixelCreate(255, 255, 255, 255, BIMAGE_U8);
    for(i = 0; i < 256; i++){
        double x = (h.bucket[i] - mn) * 255.0 / (mx - mn);
        for(n = 1; n < x; n++){
            bimageSetPixel(im, i, im->height-n, px);
        }
    }

    return im;
}
