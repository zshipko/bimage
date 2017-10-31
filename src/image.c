#include "bimage.h"

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <pthread.h>

#define BIMAGE_PIXEL_INIT bimagePixelCreate(0, 0, 0, 0, BIMAGE_UNKNOWN)

/* BIMAGE TYPE */

uint32_t
bimageTypeMax(BIMAGE_TYPE t)
{
    switch(bimageTypeDepth(t)){
    case BIMAGE_U8:
        return 0xFF;
    case BIMAGE_U16:
        return 0xFFFF;
    case BIMAGE_U32:
        return 0xFFFFFFFF;
    case BIMAGE_F32:
        return 1;
    default:
        return 0;
    }
}

/* BIMAGE */

bimage*
bimageCreateWithData (uint32_t width, uint32_t height, BIMAGE_TYPE t, void *data, bool owner, bool ondisk)
{
    if (!data){
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

size_t
bimageDepthSize(BIMAGE_DEPTH d)
{
    switch (d){
    case BIMAGE_U8: return 8;
    case BIMAGE_U16: return 16;
    case BIMAGE_U32: return 32;
    case BIMAGE_F32: return 32;
    default: return 0;
    }
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

#define MMAP_HEADER_SIZE sizeof(uint32_t) * 2 + sizeof(BIMAGE_TYPE) + 4

bimage*
bimageCreateOnDiskFd (int fd, uint32_t width, uint32_t height, BIMAGE_TYPE t)
{
    bool loadFile = (width == 0 && height == 0) || bimageTypeDepth(t) == BIMAGE_UNKNOWN;
    if (loadFile){
        char hdr[4];
        if (read(fd, &hdr, 4) != 4){
            return NULL;
        }

        if (strncmp(hdr, "BIMG", 4) != 0){
            return NULL;
        }

        if (read(fd, &t, sizeof(BIMAGE_TYPE)) != sizeof(BIMAGE_TYPE)){
            return NULL;
        }

        if (read(fd, &width, sizeof(uint32_t)) != sizeof(uint32_t)){
            return NULL;
        }

        if (read(fd, &height, sizeof(uint32_t)) != sizeof(int32_t)){
            return NULL;
        }

        if (width == 0 || height == 0 || bimageTypeDepth(t) == BIMAGE_UNKNOWN || bimageTypeChannels(t) == 0){
            return NULL;
        }

        if (lseek(fd, 0, SEEK_END) < width * height * bimageDepthSize(bimageTypeDepth(t)) * bimageTypeChannels(t)){
            return NULL;
        }

        lseek(fd, 0, SEEK_SET);
    }

    // Write header for new images
    if (!loadFile){
        if (write(fd, "BIMG", 4) != 4){
            return NULL;
        }

        if (write(fd, &t, sizeof(BIMAGE_TYPE)) != sizeof(BIMAGE_TYPE)){
            return NULL;
        }

        if(write(fd, &width, sizeof(uint32_t)) != sizeof(uint32_t)){
            return NULL;
        }

        if (write(fd, &height, sizeof(uint32_t)) != sizeof(uint32_t)){
            return NULL;
        }

        // Allocate enough memory on disk
        lseek(fd, bimageTotalSize(width, height, t), SEEK_SET);
        if (write(fd, "\0", 1) != 1){
            return NULL;
        }
    }

    void *data = mmap(NULL, bimageTotalSize(width, height, t) + MMAP_HEADER_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, false);

    // Check data
    if (!data){
        return NULL;
    }

    return bimageCreateWithData(width, height, t, data + MMAP_HEADER_SIZE, true, true);
}

bimage*
bimageCreateOnDisk (const char *filename, uint32_t width, uint32_t height, BIMAGE_TYPE t)
{
    bool loadExisting = (width == 0 && height == 0) || t == BIMAGE_UNKNOWN;
    int fd = open(filename,  loadExisting ? O_RDWR : O_RDWR|O_CREAT, 0655);
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

static void
bimageReleaseData(bimage* im)
{
    if (im->data){
        if (im->ondisk){
            munmap(im->data-MMAP_HEADER_SIZE, bimageTotalSize(im->width, im->height, im->type));
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
    if (im && *im){
        bimageRelease(*im);
        *im = NULL;
    }
}

BIMAGE_STATUS
bimageMapToDisk(const char *filename, bimage** im)
{
    bimage *tmp = bimageCreateOnDisk(filename, (*im)->width, (*im)->height, (*im)->type);
    if (!tmp){
        return BIMAGE_ERR;
    }

    // Copy pixel data to temporary image
    memcpy(tmp->data, (*im)->data, bimageTotalSize(tmp->width, tmp->height, tmp->type));

    // Copy data to destination image
    bimageReleaseData((*im));
    (*im)->owner = true;
    (*im)->ondisk = true;
    (*im)->data = tmp->data;

    // Free temporary image
    tmp->owner = false;
    bimageRelease(tmp);

    return BIMAGE_OK;
}

bimage*
bimageConsume(bimage **dst, bimage *src)
{
    bimageRelease(*dst);
    *dst = src;
    return *dst;
}

BIMAGE_STATUS
bimageGetPixelUnsafe(bimage *im, uint32_t x, uint32_t y, bimagePixel *p)
{
    if (bimageTypeChannels(im->type) > 4){
        return BIMAGE_ERR;
    }

    int i;
    int64_t offs = bimageIndex(im, x, y);

    // Set bimagePixel depth
    p->depth = bimageTypeDepth(im->type);
    p->data.f[3] = bimageTypeMax(im->type);

    for (i = 0; i < bimageTypeChannels(im->type); i++){
        switch (p->depth){
        case BIMAGE_U8:
            p->data.f[i] = (float)bimageAt(im, offs+i, uint8_t);
            break;
        case BIMAGE_U16:
            p->data.f[i] = (float)bimageAt(im, offs+i, uint16_t);
            break;
        case BIMAGE_U32:
            p->data.f[i] = (float)bimageAt(im, offs+i, uint32_t);
            break;
        case BIMAGE_F32:
            p->data.f[i] = bimageAt(im, offs+i, float);
            break;
        default:
            return BIMAGE_ERR;
        }
    }

    // Grayscale bimagePixels should have the same value for RGB channels
    if (bimageTypeChannels(im->type) == 1){
        p->data.f[1] = p->data.f[2] = p->data.f[0];
    }

    return BIMAGE_OK;
}



BIMAGE_STATUS
bimageGetPixel(bimage *im, uint32_t x, uint32_t y, bimagePixel *p)
{
    if (!p){
        return BIMAGE_ERR;
    }

    if (!bimageBoundsCheck(im, x, y)){
        bimagePixelZero(p, BIMAGE_UNKNOWN);
        return BIMAGE_ERR;
    }

    return bimageGetPixelUnsafe(im, x, y, p);
}

BIMAGE_STATUS
bimageSetPixelUnsafe(bimage *im, uint32_t x, uint32_t y, bimagePixel p)
{
    int i;
    int64_t offs = bimageIndex(im, x, y);
    for (i = 0; i < bimageTypeChannels(im->type) % 5; i++){
        switch (bimageTypeDepth(im->type)){
        case BIMAGE_U8:
            bimageAt(im, offs+i, uint8_t) = (uint8_t)p.data.f[i];
            break;
        case BIMAGE_U16:
            bimageAt(im, offs+i, uint16_t) = (uint16_t)p.data.f[i];
            break;
        case BIMAGE_U32:
            bimageAt(im, offs+i, uint32_t) = (uint32_t)p.data.f[i];
            break;
        case BIMAGE_F32:
            bimageAt(im, offs+i, float) = p.data.f[i];
            break;
        default:
            return BIMAGE_ERR;
        }
    }

    return BIMAGE_OK;
}


BIMAGE_STATUS
bimageSetPixel(bimage *im, uint32_t x, uint32_t y, bimagePixel p)
{
    bimagePixel q;
    bimagePixelZero(&q, p.depth);

    if (!bimageBoundsCheck(im, x, y)){
        return BIMAGE_ERR;
    }

    // Set bimagePixel depth based on image
    if (bimagePixelConvertDepth(&q, p, bimageTypeDepth(im->type)) != BIMAGE_OK){
        return BIMAGE_ERR;
    }

    return bimageSetPixelUnsafe(im, x, y, q);
}

/* bimageConvertDepth converts between 8, 16 and 32 bit images */
bimage*
bimageConvertDepth(bimage *dst, bimage *im, BIMAGE_DEPTH depth)
{
    bimagePixel px = BIMAGE_PIXEL_INIT, pdst = BIMAGE_PIXEL_INIT;
    bimage* im2 = BIMAGE_CREATE_DEST(dst, im->width, im->height, depth | bimageTypeChannels(im->type));
    if (!im2){
        return NULL;
    }

    bimageIterAll(im, x, y){
        if (bimageGetPixelUnsafe(im, x, y, &px) == BIMAGE_ERR
                || bimagePixelConvertDepth(&pdst, px, depth) != BIMAGE_ERR){
            bimageSetPixel(im2, x, y, pdst);
        } else {
            break;
        }
    }

    return im2;
}

/* bimageConvertChannels converts between 1, 3, and 4 channel images
 * */
bimage*
bimageConvertChannels(bimage* dst, bimage* im, BIMAGE_CHANNEL nchannels)
{
    bimage* im2 = BIMAGE_CREATE_DEST(dst, im->width, im->height, bimageTypeDepth(im->type) | nchannels);
    if (!im2){
        return NULL;
    }

    bimagePixel px = BIMAGE_PIXEL_INIT;
    bimageIterAll(im, x, y){
        if (bimageGetPixelUnsafe(im, x, y, &px) != BIMAGE_ERR){
            bimageSetPixel(im2, x, y, px);
        } else {
            break;
        }
    }

    return im2;
}


bimage*
bimageCrop (bimage* dst, bimage* im, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    bimage* im2 = BIMAGE_CREATE_DEST(dst, w, h, im->type);
    if (!im2){
        return NULL;
    }

    bimagePixel px = BIMAGE_PIXEL_INIT;
    bimageIter(im, i, j, x, y, w, h, 1, 1){
        if (bimageGetPixel(im, i, j, &px) == BIMAGE_OK){
            bimageSetPixel(im2, i-x, j-y, px);
        } else {
            break;
        }
    }

    return im2;
}

void
bimageCopyTo (bimage* dst, bimage* src, uint32_t offs_x, uint32_t offs_y)
{
    if (offs_x >= dst->width || offs_y >= dst->height){
        return;
    }

    bimagePixel px = BIMAGE_PIXEL_INIT;
    bimageIterAll(src, x, y){
        bimageGetPixelUnsafe(src, x, y, &px);
        bimageSetPixel(dst, x+offs_x, y+offs_y, px);
    }
}

void
bimageAdjustGamma (bimage* im, float g)
{
    int i, c = bimageTypeChannels(im->type);
    c = c > 3 ? 3 : c; // Ignore alpha channel
    float mx = (float)bimageTypeMax(im->type);

    bimagePixel px = BIMAGE_PIXEL_INIT;
    bimageIterAll(im, x, y){
        bimageGetPixelUnsafe(im, x, y, &px);
        for(i = 0; i < c; i++){
            px.data.f[i] = mx * pow((px.data.f[i]/mx), (1.0/g));
        }
        bimageSetPixel(im, x, y, px);
    }
}

bimagePixel
bimageAverageInRect(bimage* im, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    int64_t n = -1;
    bimagePixel px = BIMAGE_PIXEL_INIT,
                dst = bimagePixelCreate(0, 0, 0, 0, bimageTypeDepth(im->type));
    bimageIter(im, i, j, x, y, w, h, 1, 1){
        if (bimageGetPixel(im, i, j, &px) == BIMAGE_OK){
            bimagePixelAdd(&dst, px);
            n += 1;
        }
    }

    bimagePixelDiv(&dst, bimagePixelCreate(n, n, n, n, bimageTypeDepth(im->type)));
    return dst;
}

bimage* bimageRandom(bimage* dst, uint32_t w, uint32_t h, BIMAGE_TYPE t)
{
    bimage* im = BIMAGE_CREATE_DEST(dst, w, h, t);
    if (!im){
        return NULL;
    }

    bimageIterAll(im, x, y){
        bimageSetPixel(im, x, y, bimagePixelRandom(bimageTypeDepth(t)));
    }

    return im;
}

#ifndef BIMAGE_NO_PTHREAD

typedef void (*bimageParallelFn)(uint32_t, uint32_t, bimagePixel *);

struct bimageParallelIterator {
    uint32_t x0, y0, x1, y1;
    bimage *image;
    bimageParallelFn f;
};

void *bimageParallelWrapper(void *_iter){
    struct bimageParallelIterator *iter =
        (struct bimageParallelIterator*)_iter;
    bimagePixel px;

    for (uint32_t j = iter->y0; j < iter->y1; j++){
        for(uint32_t i = iter->x0; i < iter->x1; i++){
            if (bimageGetPixel(iter->image, i, j, &px) == BIMAGE_OK){
                iter->f(i, j, &px);
                bimageSetPixel(iter->image, i, j, px);
            }
        }
    }

    pthread_exit(NULL);
    return NULL;
}

BIMAGE_STATUS bimageParallel(bimage* im, bimageParallelFn fn, int nthreads){
    pthread_t threads[nthreads];
    int tries = 1;
    uint32_t width, height;

    width = im->width / nthreads;
    height = im->height / nthreads;

    for (uint32_t x = 0; x < nthreads; x++){
        struct bimageParallelIterator iter;
        iter.x0 = width * x;
        iter.x1 = width;
        iter.y0 = height * x;
        iter.y1 = height;
        iter.image = im;
        iter.f = fn;
        if (pthread_create(&threads[x], NULL, bimageParallelWrapper, &iter) != 0){
            if (tries < 10){
                x -= 1;
                tries += 1;
            } else {
                return BIMAGE_ERR;
            }
        } else {
            tries = 0;
        }
    }

    for(int n = 0; n < nthreads; n++){
        // Maybe do something if this fails?
        pthread_join(threads[n], NULL);
    }

    return BIMAGE_OK;
}

#endif // BIMAGE_NO_PTHREAD
