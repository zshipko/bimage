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
bimageMakeType(BIMAGE_CHANNEL channels, BIMAGE_DEPTH depth, BIMAGE_TYPE *dst)
{
    switch (channels) {
    case BIMAGE_GRAY:
        switch (depth) {
        case BIMAGE_U8: *dst = BIMAGE_GRAY8; break;
        case BIMAGE_U16: *dst = BIMAGE_GRAY16; break;
        case BIMAGE_U32: *dst = BIMAGE_GRAY32; break;
        default: return BIMAGE_ERR;
        }
        break;
    case BIMAGE_RGB:
        switch (depth) {
        case BIMAGE_U8: *dst = BIMAGE_RGB24; break;
        case BIMAGE_U16: *dst = BIMAGE_RGB48; break;
        case BIMAGE_U32: *dst = BIMAGE_RGB96; break;
        default: return BIMAGE_ERR;
        }
        break;
    case BIMAGE_RGBA:
        switch (depth) {
        case BIMAGE_U8: *dst = BIMAGE_RGBA32; break;
        case BIMAGE_U16: *dst = BIMAGE_RGBA64; break;
        case BIMAGE_U32: *dst = BIMAGE_RGBA128; break;
        default: return BIMAGE_ERR;
        }
        break;
    default:
        return BIMAGE_ERR;
    }

    return BIMAGE_OK;
}

BIMAGE_CHANNEL
bimageTypeChannels(BIMAGE_TYPE t)
{
    switch(t){
    case BIMAGE_GRAY8:
    case BIMAGE_GRAY16:
    case BIMAGE_GRAY32:
        return BIMAGE_GRAY;
    case BIMAGE_RGB24:
    case BIMAGE_RGB48:
    case BIMAGE_RGB96:
        return BIMAGE_RGB;
    case BIMAGE_RGBA32:
    case BIMAGE_RGBA64:
    case BIMAGE_RGBA128:
        return BIMAGE_RGBA;
    default:
        return 0;
    }
}

int64_t
bimageTypeMax(BIMAGE_TYPE t)
{
    switch(t){
    case BIMAGE_GRAY8:
    case BIMAGE_RGB24:
    case BIMAGE_RGBA32:
        return 0xFF;
    case BIMAGE_GRAY16:
    case BIMAGE_RGB48:
    case BIMAGE_RGBA64:
        return 0xFFFF;
    case BIMAGE_GRAY32:
    case BIMAGE_RGB96:
    case BIMAGE_RGBA128:
        return 0xFFFFFFFF;
    default:
        return 0;
    }
}


BIMAGE_DEPTH
bimageTypeDepth(BIMAGE_TYPE t)
{
    switch(t){
    case BIMAGE_GRAY8:
    case BIMAGE_RGB24:
    case BIMAGE_RGBA32:
        return BIMAGE_U8;
    case BIMAGE_GRAY16:
    case BIMAGE_RGB48:
    case BIMAGE_RGBA64:
        return BIMAGE_U16;
    case BIMAGE_GRAY32:
    case BIMAGE_RGB96:
    case BIMAGE_RGBA128:
        return BIMAGE_U32;
    default:
        return BIMAGE_UNKNOWN;
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
bimageGetPixelUnsafe(bimage *im, uint32_t x, uint32_t y, bpixel *p)
{
    int channels = bimageTypeChannels(im->type), i;
    int64_t offs = bimageIndex(im, x, y);

    // Set bpixel depth
    p->depth = bimageTypeDepth(im->type);
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

    // Grayscale bpixels should have the same value for RGB channels
    if (channels == 1){
        p->data[1] = p->data[2] = p->data[0];
    }

    return BIMAGE_OK;
}



BIMAGE_STATUS
bimageGetPixel(bimage *im, uint32_t x, uint32_t y, bpixel *p)
{
    if (im->width <= x || im->height <= y){
        return BIMAGE_ERR;
    }

    return bimageGetPixelUnsafe(im, x, y, p);
}

BIMAGE_STATUS
bimageSetPixelUnsafe(bimage *im, uint32_t x, uint32_t y, bpixel p)
{

    BIMAGE_CHANNEL channels = bimageTypeChannels(im->type), i;
    int64_t offs = bimageIndex(im, x, y);

    for (i = 0; i < channels; i++){
        switch (p.depth){
        case BIMAGE_U8:
            bimageAt(im, offs+i, uint8_t) = (uint8_t)p.data[i];
            break;
        case BIMAGE_U16:
            bimageAt(im, offs+i, uint16_t) = (uint16_t)p.data[i];
            break;
        case BIMAGE_U32:
            bimageAt(im, offs+i, uint32_t) = (uint32_t)p.data[i];
            break;
        default:
            return BIMAGE_ERR;
        }
    }

    return BIMAGE_OK;
}


BIMAGE_STATUS
bimageSetPixel(bimage *im, uint32_t x, uint32_t y, bpixel p)
{

    // Bounds check
    if (!im || im->width <= x || im->height <= y){
        return BIMAGE_ERR;
    }

    // Set bpixel depth based on image
    BIMAGE_DEPTH depth = bimageTypeDepth(im->type);
    if (p.depth < 0){
        p.depth = depth;
    } else if (p.depth != depth) {
        if (bpixelConvertDepth(p, bimageTypeDepth(im->type), &p) != BIMAGE_OK){
            return BIMAGE_ERR;
        }
    }

    return bimageSetPixelUnsafe(im, x, y, p);
}

/* bimageConvertDepth converts between 8, 16 and 32 bit images */
bimage*
bimageConvertDepth(bimage **dst, bimage *im, BIMAGE_DEPTH depth)
{
    bpixel px, pdst;
    BIMAGE_TYPE t;
    if (bimageMakeType(bimageTypeChannels(im->type), depth, &t) == BIMAGE_ERR){
        return NULL;
    }

    bimage* im2 = BIMAGE_CREATE_DEST(dst, im->width, im->height, t);
    if (!im2){
        return NULL;
    }

    bimageIterAll(im, x, y){
        if (bimageGetPixelUnsafe(im, x, y, &px) == BIMAGE_ERR
                || bpixelConvertDepth(px, depth, &pdst) != BIMAGE_ERR){
            bimageSetPixel(im2, x, y, pdst);
        } else {
            break;
        }
    }

    BIMAGE_RETURN_DEST(dst, im2);
}

/* bimageConvertChannels converts between 1, 3, and 4 channel images
 * */
bimage*
bimageConvertChannels(bimage** dst, bimage* im, BIMAGE_CHANNEL nchannels)
{
    BIMAGE_TYPE t;

    if (bimageMakeType(nchannels, bimageTypeDepth(im->type), &t) == BIMAGE_ERR){
        return NULL;
    }

    bimage* im2 = BIMAGE_CREATE_DEST(dst, im->width, im->height, t);
    if (!dst){
        return NULL;
    }

    bpixel px;
    bimageIterAll(im, x, y){
        if (bimageGetPixelUnsafe(im, x, y, &px) != BIMAGE_ERR){
            bimageSetPixel(im2, x, y, px);
        } else {
            break;
        }
    }

    BIMAGE_RETURN_DEST(dst, im2);
}


bimage*
bimageCrop (bimage** dst, bimage* im, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    bimage* im2 = BIMAGE_CREATE_DEST(dst, w, h, im->type);
    if (!dst){
        return NULL;
    }

    bpixel px;
    bimageIter(im, i, j, x, y, w, h){
        if (bimageGetPixel(im, i, j, &px) != BIMAGE_ERR){
            bimageSetPixel(im2, i-x, j-y, px);
        } else {
            break;
        }
    }

    BIMAGE_RETURN_DEST(dst, im2);
}

void
bimageCopyTo (bimage* dst, bimage* src, uint32_t offs_x, uint32_t offs_y)
{
    if (offs_x >= dst->width || offs_y >= dst->height){
        return;
    }

    bpixel px;
    bimageIterAll(src, x, y){
        bimageGetPixelUnsafe(src, x, y, &px);
        bimageSetPixel(dst, x+offs_x, y+offs_y, px);
    }
}

void
bimageAdjustGamma (bimage* im, float g)
{
    int i, c = bimageTypeChannels(im->type);
    c = c > 3 ? 3 : c;
    float mx = (float)bimageTypeMax(im->type);

    bimageIterAll(im, x, y){
        bpixel px;

        bimageGetPixelUnsafe(im, x, y, &px);
        for(i = 0; i < c; i++){
            px.data[i] = mx * pow((px.data[i]/mx), (1.0/g));
        }
        bimageSetPixel(im, x, y, px);
    }
}
