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
    uint64_t width, height;
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

#define bimageMemoryAlloc(n) calloc(n, 1)
#define bimageMemoryFree(x) if(x) free(x)
#define bimageTotalSize(w, h, t) w * h * (int64_t)bimageTypeSize(t) * (int64_t)bimageTypeChannels(t)
#define bimageIndex(im, x, y) y * bimageTypeChannels(im->type) * im->width + x * bimageTypeChannels(im->type)
#define bimageAt(im, index, t) (((t*)im->data)[index])

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
bimageCreateWithData (int64_t width, int64_t height, BIMAGE_TYPE t, void *data, bool owner, bool ondisk);

bimage*
bimageCreate (int64_t width, int64_t height, BIMAGE_TYPE t);

bimage*
bimageCreateOnDisk (char* filename, int64_t width, int64_t height, BIMAGE_TYPE t);

void
bimageRelease(bimage* im);

BIMAGE_STATUS
bimageGetPixel(bimage* im, int64_t x, int64_t y, pixel *p);

BIMAGE_STATUS
bimageSetPixel(bimage* im, int64_t x, int64_t y, pixel p);

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

/* RESIZE */

bimage*
bimageResize(bimage *im, int32_t width, int32_t height);


#endif // __BIMAGE_HEADER_GUARD
