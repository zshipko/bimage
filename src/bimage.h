#ifndef __BIMAGE_HEADER_GUARD
#define __BIMAGE_HEADER_GUARD

#include <stdint.h>
#include <stdbool.h>

typedef enum BIMAGE_STATUS {
    BIMAGE_ERR,
    BIMAGE_OK
} BIMAGE_STATUS;

typedef enum BIMAGE_TYPE {
    GRAY8,
    GRAY16,
    GRAY32,
    RGB24,
    RGB48,
    RGB96,
    RGBA32,
    RGBA64,
    RGBA128
} BIMAGE_TYPE;

typedef struct bimage {
    uint32_t width, height;
    BIMAGE_TYPE type;
    void* data;
    bool owner;
    bool ondisk;
} bimage;

typedef struct pixel {
    float data[4];
    int8_t depth;
} pixel;

#define U8  8
#define U16 16
#define U32 32

#define bAlloc(n) calloc(n, 1)
#define bFree(x) if(x) free(x)
#define bimageTotalSize(w, h, t) (int64_t)w * (int64_t)h * (int64_t)bimageTypeSize(t) * (int64_t)bimageTypeChannels(t)
#define bimageIndex(im, x, y) y * bimageTypeChannels(im->type) * im->width + x * bimageTypeChannels(im->type)
#define bimageAt(im, index, t) (((t*)im->data)[index])
#define bimageIter(im, x, y, _x, _y, _w, _h) \
    int32_t x, y; \
    for(y = _y; y < im->height && y < _y + _h; y++) \
        for(x = _x; x < im->width && x < _x + _w; x++)
#define bimageIterAll(im, x, y) \
    int32_t x, y; \
    for(y = 0; y < im->height; y++) \
        for(x = 0; x < im->width; x++)


/* BIMAGE */

pixel
pixelCreate (float r, float g, float b, float a, int depth);

BIMAGE_STATUS
pixelConvertDepth (pixel a, uint8_t depth, pixel *dst);

BIMAGE_STATUS
bimageTypeFromChannelsAndDepth(int8_t channels, int8_t depth, BIMAGE_TYPE *dst);

uint8_t
bimageTypeChannels(BIMAGE_TYPE t);

int64_t
bimageTypeMax(BIMAGE_TYPE t);

int8_t
bimageTypeSize(BIMAGE_TYPE t);

bimage*
bimageCreateWithData (uint32_t width, uint32_t height, BIMAGE_TYPE t, void *data, bool owner, bool ondisk);

bimage*
bimageCreate (uint32_t width, uint32_t height, BIMAGE_TYPE t);

bimage*
bimageCreateOnDiskFd (int fd, uint32_t width, uint32_t height, BIMAGE_TYPE t);

bimage*
bimageCreateOnDisk (char* filename, uint32_t width, uint32_t height, BIMAGE_TYPE t);

void
bimageRelease(bimage* im);

void
bimageDestroy(bimage** im);

bimage*
bimageConsume(bimage **dst, bimage *src);

BIMAGE_STATUS
bimageGetPixel(bimage* im, uint32_t x, uint32_t y, pixel *p);

BIMAGE_STATUS
bimageSetPixel(bimage* im, uint32_t x, uint32_t y, pixel p);

/* TIFF */

bimage*
bimageOpenTIFF(const char* filename);

BIMAGE_STATUS
bimageSaveTIFF(bimage* im, const char* filename);

/* IO */

bimage*
bimageOpen8(const char* filename);

bimage*
bimageOpen16(const char* filename);

bimage*
bimageOpen32(const char* filename);

BIMAGE_STATUS
bimageSave(bimage *im, const char *filename);

bimage*
bimageConvertDepth(bimage *im, int8_t depth);

/* RESIZE */

bimage*
bimageResize(bimage *im, int32_t width, int32_t height);

/* FILTER */

bimage*
bimageGrayscale(bimage *im);

bimage*
bimageFilter(bimage *im, float *K, int Ks, float divisor, float offset);


#endif // __BIMAGE_HEADER_GUARD
