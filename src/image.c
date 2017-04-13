#include "bimage.h"

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <string.h>

/* BIMAGE TYPE */
BIMAGE_STATUS
bimageTypeFromChannelsAndDepth(int8_t channels, int8_t depth, BIMAGE_TYPE *dst)
{
    switch (channels) {
    case 1:
        switch (depth) {
        case 8: *dst = GRAY8; break;
        case 16: *dst = GRAY16; break;
        case 32: *dst = GRAY32; break;
        default: return BIMAGE_ERR;
        }
        break;
    case 3:
        switch (depth) {
        case 8: *dst = RGB24; break;
        case 16: *dst = RGB48; break;
        case 32: *dst = RGB96; break;
        default: return BIMAGE_ERR;
        }
        break;
    case 4:
        switch (depth) {
        case 8: *dst = RGBA32; break;
        case 16: *dst = RGBA64; break;
        case 32: *dst = RGBA128; break;
        default: return BIMAGE_ERR;
        }
        break;
    default:
        return BIMAGE_ERR;
    }

    return BIMAGE_OK;
}

uint8_t
bimageTypeChannels(BIMAGE_TYPE t)
{
    switch(t){
    case GRAY8:
    case GRAY16:
    case GRAY32:
        return 1;
    case RGB24:
    case RGB48:
    case RGB96:
        return 3;
    case RGBA32:
    case RGBA64:
    case RGBA128:
        return 4;
    }
}

int64_t
bimageTypeMax(BIMAGE_TYPE t)
{
    switch(t){
    case GRAY8:
    case RGB24:
    case RGBA32:
        return 0xFF;
    case GRAY16:
    case RGB48:
    case RGBA64:
        return 0xFFFF;
    case GRAY32:
    case RGB96:
    case RGBA128:
        return 0xFFFFFFFF;
    }
}


int8_t
bimageTypeSize(BIMAGE_TYPE t)
{
    switch(t){
    case GRAY8:
    case RGB24:
    case RGBA32:
        return 8;
    case GRAY16:
    case RGB48:
    case RGBA64:
        return 16;
    case GRAY32:
    case RGB96:
    case RGBA128:
        return 32;
    }
}

/* BIMAGE */

bimage*
bimageCreateWithData (uint32_t width, uint32_t height, BIMAGE_TYPE t, void *data, bool owner, bool ondisk)
{

    if (data == NULL){
        return NULL;
    }

    if (width == 0 || height == 0){
        if (owner){
            bFree(data);
        }
        return NULL;
    }

    bimage *im = bAlloc(sizeof(bimage));
    if (!im){
        if (owner){
            bFree(data);
        }
        return NULL;
    }

    im->width = width;
    im->height = height;
    im->type = t;
    im->data = data;
    im->owner = owner;
    im->ondisk = ondisk;

    return im;
}

bimage*
bimageCreate (uint32_t width, uint32_t height, BIMAGE_TYPE t)
{
    if (width == 0 || height == 0){
        return NULL;
    }

    void* data = bAlloc(bimageTotalSize(width, height, t));
    if (!data){
        return NULL;
    }

    return bimageCreateWithData(width, height, t, data, true, false);
}

bimage*
bimageCreateOnDiskFd (int fd, uint32_t width, uint32_t height, BIMAGE_TYPE t)
{
    // Allocate enough memory on disk
    lseek(fd, bimageTotalSize(width, height, t), SEEK_SET);

    void *data = mmap(NULL, bimageTotalSize(width, height, t), PROT_READ|PROT_WRITE, MAP_SHARED, fd, false);

    // Check data
    if (!data){
        return NULL;
    }

    return bimageCreateWithData(width, height, t, data, true, true);
}

bimage*
bimageCreateOnDisk (char *filename, uint32_t width, uint32_t height, BIMAGE_TYPE t)
{
    int fd = open(filename, O_RDWR|O_CREAT, 0655);
    if (fd < 0){
        return NULL;
    }

    bimage *im = bimageCreateOnDiskFd(fd, width, height, t);

    close(fd);
    return im;
}

bool
bimageIsValid(bimage *im)
{
    return im && im->data && im->width > 0 && im->height > 0;
}

static inline void
bimageReleaseData(bimage* im)
{
    if (im->data){
        if (im->ondisk){
            munmap(im->data, bimageTotalSize(im->width, im->height, im->type));
        } else {
            bFree(im->data);
        }
        im->data = NULL;
    }
}

void
bimageRelease(bimage *im)
{
    if (im){
        if (im->owner){
            bimageReleaseData(im);
        }

        bFree(im);
    }
}

void
bimageDestroy(bimage **im)
{
    if (im){
        bimageRelease(*im);
        *im = NULL;
    }
}

bimage*
bimageConsume(bimage **dst, bimage *src)
{
    bimageRelease(*dst);
    *dst = src;
    return *dst;
}


BIMAGE_STATUS
bimageGetPixel(bimage *im, uint32_t x, uint32_t y, pixel *p)
{
    if (im->width <= x || im->height <= y){
        return BIMAGE_ERR;
    }

    int channels = bimageTypeChannels(im->type), i;
    int64_t offs = bimageIndex(im, x, y);
    float mx = (float)bimageTypeMax(im->type);

    // Set pixel depth
    p->depth = bimageTypeSize(im->type);
    p->data[3] = bimageTypeMax(im->type);

    for (i = 0; i < channels; i++){
        switch (p->depth){
        case 8:
            p->data[i] = (float)bimageAt(im, offs+i, uint8_t);
            break;
        case 16:
            p->data[i] = (float)bimageAt(im, offs+i, uint16_t);
            break;
        case 32:
            p->data[i] = (float)bimageAt(im, offs+i, uint32_t);
            break;
        default:
            return BIMAGE_ERR;
        }
    }

    // Grayscale pixels should have the same value for RGB channels
    if (channels == 1){
        p->data[1] = p->data[2] = p->data[0];
    }

    return BIMAGE_OK;
}

BIMAGE_STATUS
bimageSetPixel(bimage *im, uint32_t x, uint32_t y, pixel p)
{

    // Bounds check
    if (im->width <= x || im->height <= y){
        return BIMAGE_ERR;
    }

    // Set pixel depth based on image
    int size = bimageTypeSize(im->type);
    if (p.depth < 0){
        p.depth = size;
    } else if (p.depth != size) {
        if (pixelConvertDepth(p, bimageTypeSize(im->type), &p) != BIMAGE_OK){
            return BIMAGE_ERR;
        }
    }

    int8_t channels = bimageTypeChannels(im->type), i;
    int64_t offs = bimageIndex(im, x, y);
    float mx = (float)bimageTypeMax(im->type);

    for (i = 0; i < channels; i++){
        switch (p.depth){
        case 8:
            bimageAt(im, offs+i, uint8_t) = (uint8_t)p.data[i];
            break;
        case 16:
            bimageAt(im, offs+i, uint16_t) = (uint16_t)p.data[i];
            break;
        case 32:
            bimageAt(im, offs+i, uint32_t) = (uint32_t)p.data[i];
            break;
        default:
            return BIMAGE_ERR;
        }
    }

    return BIMAGE_OK;
}

bimage*
bimageConvertDepth(bimage *im, int8_t depth)
{
    pixel px, dst;
    BIMAGE_TYPE t;
    if (bimageTypeFromChannelsAndDepth(bimageTypeChannels(im->type), depth, &t) == BIMAGE_ERR){
        return NULL;
    }

    bimage *im2 = bimageCreate(im->width, im->height, t);
    if (!im2){
        return NULL;
    }

    bimageIterAll(im, x, y){
        bimageGetPixel(im, x, y, &px);
        pixelConvertDepth(px, depth, &dst);
        bimageSetPixel(im2, x, y, dst);
    }

    return im2;
}



