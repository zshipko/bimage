#include "bimage.h"

#include <stdlib.h>
#include <stdio.h>

#define HASH_SIZE 8

uint64_t bimageHash(bimage *im)
{
    uint64_t hash = 0UL; // Output
    bpixel px;           // Current pixel
    bpixel apx;          // Average pixel
    int i, j, n = 0;

    apx = bpixelCreate(0, 0, 0, 0, bimageTypeSize(im->type));

    bimage *sm = bimageResize(im, HASH_SIZE, HASH_SIZE);
    if (!sm){
        return 0UL;
    }

    bimageConsume(&sm, bimageGrayscale(sm));
    if (!sm){
        return 0UL;
    }

    for(j = 0; j < HASH_SIZE; j++){
        for(i = 0; i < HASH_SIZE; i++){
            // Set current pixel
            if (bimageGetPixel(sm, i, j, &px) == BIMAGE_ERR){
                continue;
            }

            // Compare current pixel against the average
            if (px.data[0] > apx.data[0]){
                hash |= 1<<n;
            } else {
                hash &= ~(1<<n);
            }

            apx.data[0] = px.data[0];
            n = n + 1;
        }
    }

    bimageDestroy(&sm);
    return hash;
}

int bimageHashDiff (uint64_t a, uint64_t b)
{
    int i, n = 0;
    for(i = 0; i < 64; i++){
        n += llabs((int64_t)(((a >> i) & 1) - ((b >> i) & 1)));
    }
    return n;
}

void bimageHashString(char dst[9], uint64_t hash)
{
    snprintf(dst, 8, "%08Lx\n", hash);
    dst[9] = '\0';
}