#ifndef __BIMAGE_HEADER_GUARD
#define __BIMAGE_HEADER_GUARD

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef enum BIMAGE_STATUS {
    BIMAGE_ERR,
    BIMAGE_OK
} BIMAGE_STATUS;

typedef enum BIMAGE_TYPE {
    BIMAGE_GRAY8,
    BIMAGE_GRAY16,
    BIMAGE_GRAY32,
    BIMAGE_RGB24,
    BIMAGE_RGB48,
    BIMAGE_RGB96,
    BIMAGE_RGBA32,
    BIMAGE_RGBA64,
    BIMAGE_RGBA128
} BIMAGE_TYPE;

typedef enum BIMAGE_CHANNEL {
    BIMAGE_GRAY = 1,
    BIMAGE_RGB = 3,
    BIMAGE_RGBA = 4
} BIMAGE_CHANNEL;

typedef enum BIMAGE_DEPTH {
    BIMAGE_UNKNOWN = -1,
    BIMAGE_U8 = 8,
    BIMAGE_U16 = 16,
    BIMAGE_U32 = 32
} BIMAGE_DEPTH;

typedef struct bimage {
    uint32_t width, height;
    BIMAGE_TYPE type;
    void* data;
    bool owner;
    bool ondisk;
} bimage;

typedef struct bpixel {
    float data[4];
    BIMAGE_DEPTH depth;
} bpixel;

#define bAlloc(n) calloc(n, 1)
#define bFree(x) if(x) free(x)
#define bimageTotalSize(w, h, t) (int64_t)w * (int64_t)h * (int64_t)bimageTypeDepth(t) * (int64_t)bimageTypeChannels(t)
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

BIMAGE_STATUS
bpixelInit(bpixel *px, float r, float g, float b, float a, BIMAGE_DEPTH depth);

bpixel
bpixelCreate (float r, float g, float b, float a, BIMAGE_DEPTH depth);

BIMAGE_STATUS
bpixelZero(bpixel *px, BIMAGE_DEPTH depth);

BIMAGE_STATUS
bpixelConvertDepth (bpixel *dst, bpixel *src, BIMAGE_DEPTH depth);

void
bpixelClamp(bpixel *p);

BIMAGE_STATUS
bimageMakeType(BIMAGE_CHANNEL channels, BIMAGE_DEPTH depth, BIMAGE_TYPE *dst);

BIMAGE_CHANNEL
bimageTypeChannels(BIMAGE_TYPE t);

int64_t
bimageTypeMax(BIMAGE_TYPE t);

BIMAGE_DEPTH
bimageTypeDepth(BIMAGE_TYPE t);

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
bimageGetPixelUnsafe(bimage* im, uint32_t x, uint32_t y, bpixel *p);

BIMAGE_STATUS
bimageSetPixelUnsafe(bimage* im, uint32_t x, uint32_t y, bpixel *p);

BIMAGE_STATUS
bimageGetPixel(bimage* im, uint32_t x, uint32_t y, bpixel *p);

BIMAGE_STATUS
bimageSetPixel(bimage* im, uint32_t x, uint32_t y, bpixel *p);

bimage*
bimageCrop(bimage** dst, bimage* im, uint32_t x, uint32_t y, uint32_t w, uint32_t h);

void
bimageCopyTo(bimage *dst, bimage *src, uint32_t x, uint32_t y);

/* TIFF */

bimage*
bimageOpenTIFF(const char* filename);

BIMAGE_STATUS
bimageSaveTIFF(bimage* im, const char* filename);

/* IO */

bimage*
bimageOpen(const char* filename);

bimage*
bimageOpen16(const char* filename);

BIMAGE_STATUS
bimageSave(bimage* im, const char* filename);

bimage*
bimageConvertDepth(bimage** dst, bimage* im, BIMAGE_DEPTH depth);

bimage*
bimageConvertChannels(bimage** dst, bimage* im, BIMAGE_CHANNEL channels);

void
bimageAdjustGamma(bimage* im, float gamma);

/* RESIZE */

bimage*
bimageResize(bimage** dst, bimage* im, int32_t width, int32_t height);

/* FILTER */

bimage*
bimageColor(bimage** dst, bimage* im, BIMAGE_CHANNEL c);

bimage*
bimageGrayscale(bimage** dst, bimage* im);

bimage*
bimageFilter(bimage** dst, bimage* im, float *K, int Ks, float divisor, float offset);

bimage*
bimageInvert(bimage** dst, bimage* im);

/* HASH */

uint64_t
bimageHash(bimage *im);

void
bimageHashString(char dst[9], uint64_t hash);

int
bimageHashDiff(uint64_t a, uint64_t b);

#define BIMAGE_CREATE_DEST(dst, w, h, t) \
    (dst ==  NULL || *dst == NULL \
        ? bimageCreate(w, h, t) \
        : ((*dst)->width >= w && (*dst)->height >= h && (*dst)->type == t ? *dst : NULL))

#define BIMAGE_RETURN_DEST(dst, im) \
    if (dst) { *dst = im; return *dst; } else { return im; }


#ifdef __cplusplus
}
#endif

#endif // __BIMAGE_HEADER_GUARD
