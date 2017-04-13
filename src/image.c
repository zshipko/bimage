#include "bimage.h"

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <string.h>

/* PIXEL */

pixel
pixelCreate (float r, float g, float b, float a, int depth)
{
    pixel px;
    bzero(&px, sizeof(pixel));
    px.depth = depth;
    px.data[0] = r;
    px.data[1] = g;
    px.data[2] = b;
    px.data[3] = a;
    return px;
}

BIMAGE_STATUS
pixelConvertDepth (pixel a, uint8_t depth, pixel *dst)
{
    if (!dst){
        return BIMAGE_ERR;
    }

    int i;
    switch (a.depth) {
    case 8:
        switch (depth) {
        case 8:
            *dst = a;
            break;
        case 16:
            for (i = 0; i < 4; i++){
                (*dst).data[i] = (uint32_t)a.data[i] << 8;
            }
            break;
        case 32:
            for (i = 0; i < 4; i++){
                (*dst).data[i] = (uint32_t)a.data[i] << 24;
            }
            break;
        default:
            return BIMAGE_ERR;
        }
        break;
    case 16:
       switch (depth) {
        case 8:
            for (i = 0; i < 4; i++){
                (*dst).data[i] = (uint32_t)a.data[i] >> 8;
            }
            break;
        case 16:
            *dst = a;
            break;
        case 32:
            for (i = 0; i < 4; i++){
                (*dst).data[i] = (uint32_t)a.data[i] << 16;
            }
            break;
        default:
            return BIMAGE_ERR;
        }
       break;
    case 32:
       switch (depth) {
        case 8:
            for (i = 0; i < 4; i++){
                (*dst).data[i] = (uint32_t)a.data[i] >> 24;
            }
            break;
        case 16:
            for (i = 0; i < 4; i++){
                (*dst).data[i] = (uint32_t)a.data[i] >> 16;
            }
            break;
        case 32:
            *dst = a;
            break;
        default:
            return BIMAGE_ERR;
        }
       break;
    default:
       return BIMAGE_ERR;
    }

    (*dst).depth = depth;
    return BIMAGE_OK;
}

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
bimageCreateWithData (int64_t width, int64_t height, BIMAGE_TYPE t, void *data, bool owner, bool ondisk)
{

    if (data == NULL){
        return NULL;
    }

    if (width == 0 || height == 0){
        if (owner){
            bimageMemoryFree(data);
        }
        return NULL;
    }

    bimage *im = bimageMemoryAlloc(sizeof(bimage));
    if (!im){
        if (owner){
            bimageMemoryFree(data);
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
bimageCreate (int64_t width, int64_t height, BIMAGE_TYPE t)
{
    if (width == 0 || height == 0){
        return NULL;
    }

    void* data = bimageMemoryAlloc(bimageTotalSize(width, height, t));
    if (!data){
        return NULL;
    }

    return bimageCreateWithData(width, height, t, data, true, false);
}

bimage*
bimageCreateOnDisk (char *filename, int64_t width, int64_t height, BIMAGE_TYPE t)
{
    int fd = open(filename, O_RDWR|O_CREAT, 0655);
    if (fd < 0){
        return NULL;
    }

    // Allocate enough memory on disk
    lseek(fd, bimageTotalSize(width, height, t), SEEK_SET);

    void *data = mmap(NULL, bimageTotalSize(width, height, t), PROT_READ|PROT_WRITE, MAP_SHARED, fd, false);

    // Close file descriptor file mmap file
    close(fd);

    // Check data
    if (!data){
        return NULL;
    }

    return bimageCreateWithData(width, height, t, data, true, true);

}

void
bimageRelease(bimage *im)
{
    if (im){
        if (im->owner){
            if (im->ondisk){
                munmap(im->data, bimageTotalSize(im->width, im->height, im->type));
            } else {
                bimageMemoryFree(im->data);
            }
        }

        bimageMemoryFree(im);
    }
}

BIMAGE_STATUS
bimageGetPixel(bimage *im, int64_t x, int64_t y, pixel *p)
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

    return BIMAGE_OK;
}

BIMAGE_STATUS
bimageSetPixel(bimage *im, int64_t x, int64_t y, pixel p)
{

    if (p.depth < 0){
        p.depth = bimageTypeSize(im->type);
    } else if (p.depth != bimageTypeSize(im->type)) {
        if (pixelConvertDepth(p, bimageTypeSize(im->type), &p) != BIMAGE_OK){
            puts("NOT OK");
        }
    }

    if (im->width <= x || im->height <= y || p.depth != bimageTypeSize(im->type)){
        return BIMAGE_ERR;
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

