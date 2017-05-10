#ifndef __BIMAGE_HEADER_GUARD
#define __BIMAGE_HEADER_GUARD

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#if defined(__has_include) && !defined(BIMAGE_NO_INTRIN)
#if (__has_include(<emmintrin.h>))
#include <emmintrin.h>
#define BIMAGE_SSE
#else
#define BIMAGE_NO_INTRIN
#endif
#else
#define BIMAGE_NO_INTRIN
#endif

typedef enum BIMAGE_STATUS {
    BIMAGE_ERR,
    BIMAGE_OK
} BIMAGE_STATUS;

typedef uint16_t BIMAGE_TYPE;

#define bimageType(d, c) (d) | (c & 0xFF)
#define bimageTypeChannels(t) ((t) & 0x00FF)
#define bimageTypeDepth(t) ((t) & 0xFF00)

typedef enum BIMAGE_CHANNEL {
    BIMAGE_GRAY = 0x0001,
    BIMAGE_RGB = 0x0003,
    BIMAGE_RGBA = 0x0004
} BIMAGE_CHANNEL;

typedef enum BIMAGE_DEPTH {
    BIMAGE_UNKNOWN = 0x0000,
    BIMAGE_U8 = 0x0100,
    BIMAGE_U16 = 0x0200,
    BIMAGE_U32 = 0x0300,
    BIMAGE_F32 = 0x0400,
} BIMAGE_DEPTH;

typedef struct bimage {
    uint32_t width, height;
    BIMAGE_TYPE type;
    void* data;
    bool owner;
    bool ondisk;
} bimage;

typedef struct bimagePixel {
    union data {
#ifdef BIMAGE_SSE
        __m128 m;
#endif
        float f[4];
    } data;
    BIMAGE_DEPTH depth;
} bimagePixel;

#define bAlloc(n) calloc(n, 1)
#define bFree(x) if(x) free(x)
#define bimageTotalSize(w, h, t) (int64_t)w * (int64_t)h * (int64_t)bimageDepthSize(bimageTypeDepth(t)) * (int64_t)bimageTypeChannels(t)
#define bimageIndex(im, x, y) y * bimageTypeChannels(im->type) * im->width + x * bimageTypeChannels(im->type)
#define bimageAt(im, index, t) (((t*)im->data)[index])
#define bimageIter(im, x, y, _x, _y, _w, _h, sx, sy) \
    int32_t x, y; \
    for(y = _y; y < im->height && y < _y + _h; y+=sy) \
        for(x = _x; x < im->width && x < _x + _w; x+=sx)
#define bimageIterAll(im, x, y) \
    int32_t x, y; \
    for(y = 0; y < im->height; y++) \
        for(x = 0; x < im->width; x++)

#define bimageBoundsCheck(im, x, y) (bimageIsValid(im) && (im)->width > (x) && (im)->height > (y) && (x) >= 0 && (y) >= 0)

typedef float (*bimagePixelOp)(float, float);
typedef void (*bimageOp)(bimage **dst, bimage*, bimage*, bimagePixelOp);

size_t
bimageDepthSize(BIMAGE_DEPTH d);

/* BIMAGE */

BIMAGE_STATUS
bimagePixelInit(bimagePixel *px, float r, float g, float b, float a, BIMAGE_DEPTH depth);

bimagePixel
bimagePixelCreate (float r, float g, float b, float a, BIMAGE_DEPTH depth);

BIMAGE_STATUS
bimagePixelZero(bimagePixel *px, BIMAGE_DEPTH depth);

BIMAGE_STATUS
bimagePixelConvertDepth (bimagePixel *dst, bimagePixel src, BIMAGE_DEPTH depth);

BIMAGE_STATUS
bimagePixelClamp(bimagePixel *p);

BIMAGE_STATUS
bimagePixelAdd(bimagePixel *p, bimagePixel q);

BIMAGE_STATUS
bimagePixelSub(bimagePixel *p, bimagePixel q);

BIMAGE_STATUS
bimagePixelMul(bimagePixel *p, bimagePixel q);

BIMAGE_STATUS
bimagePixelDiv(bimagePixel *p, bimagePixel q);

bool
bimageIsValid(bimage *im);

uint32_t
bimageTypeMax(BIMAGE_TYPE t);

bimage*
bimageCreateWithData (uint32_t width, uint32_t height, BIMAGE_TYPE t, void *data, bool owner, bool ondisk);

bimage*
bimageCreate (uint32_t width, uint32_t height, BIMAGE_TYPE t);

bimage*
bimageCreateOnDiskFd (int fd, uint32_t width, uint32_t height, BIMAGE_TYPE t);

bimage*
bimageCreateOnDisk (const char* filename, uint32_t width, uint32_t height, BIMAGE_TYPE t);

void
bimageRelease(bimage* im);

void
bimageDestroy(bimage** im);

BIMAGE_STATUS
bimageMapToDisk(const char *filename, bimage **im);

bimage*
bimageConsume(bimage **dst, bimage *src);

BIMAGE_STATUS
bimageGetPixelUnsafe(bimage* im, uint32_t x, uint32_t y, bimagePixel *p);

BIMAGE_STATUS
bimageSetPixelUnsafe(bimage* im, uint32_t x, uint32_t y, bimagePixel p);

BIMAGE_STATUS
bimageGetPixel(bimage* im, uint32_t x, uint32_t y, bimagePixel *p);

BIMAGE_STATUS
bimageSetPixel(bimage* im, uint32_t x, uint32_t y, bimagePixel p);

bimage*
bimageCrop(bimage* dst, bimage* im, uint32_t x, uint32_t y, uint32_t w, uint32_t h);

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

bimage*
bimageOpenFloat(const char* filename);

BIMAGE_STATUS
bimageSave(bimage* im, const char* filename);

bimage*
bimageConvertDepth(bimage* dst, bimage* im, BIMAGE_DEPTH depth);

bimage*
bimageConvertChannels(bimage* dst, bimage* im, BIMAGE_CHANNEL channels);

void
bimageAdjustGamma(bimage* im, float gamma);

bimagePixel
bimageAverageInRect(bimage* im, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

/* RESIZE */

bimage*
bimageResize(bimage* dst, bimage* im, int32_t width, int32_t height);

/* FILTER */

BIMAGE_STATUS
bimageAdd(bimage* dst, bimage* b);

BIMAGE_STATUS
bimageSub(bimage* dst, bimage* b);

BIMAGE_STATUS
bimageMul(bimage* dst, bimage* b);

BIMAGE_STATUS
bimageDiv(bimage* dst, bimage* b);

bimage*
bimageColor(bimage* dst, bimage* im, BIMAGE_CHANNEL c);

bimage*
bimageGrayscale(bimage* dst, bimage* im, BIMAGE_CHANNEL c);

bimage*
bimageFilter(bimage* dst, bimage* im, float *K, int Ks, float divisor, float offset);

bimage*
bimageSobelX(bimage* dst, bimage* src);

bimage*
bimageSobelY(bimage* dst, bimage* src);

bimage*
bimageSobel(bimage* dst, bimage* src);

bimage*
bimagePrewittX(bimage* dst, bimage* src);

bimage*
bimagePrewittY(bimage* dst, bimage* src);

bimage*
bimagePrewitt(bimage* dst, bimage* src);

bimage*
bimageOutline(bimage* dst, bimage* src);

bimage*
bimageSharpen(bimage* dst, bimage* src);

bimage*
bimageBlur(bimage* dst, bimage* src);

bimage*
bimageGaussianBlur(bimage* dst, bimage* src);

bimage*
bimageInvert(bimage* dst, bimage* im);

bimage*
bimageRotate(bimage* dst, bimage* im, float deg);

/* HASH */

uint64_t
bimageHash(bimage *im);

void
bimageHashString(char dst[9], uint64_t hash);

int
bimageHashDiff(uint64_t a, uint64_t b);

/* HISTOGRAM */

typedef struct bimageHistogram {
    int64_t total;
    float bucket[256];
} bimageHistogram;

BIMAGE_STATUS
bimageGetHistogram(bimage* im, bimageHistogram h[], BIMAGE_CHANNEL ch);

int
bimageHistogramMax(bimageHistogram h);
int
bimageHistogramMin(bimageHistogram h);

bimage*
bimageHistogramImage(bimageHistogram h);

#define BIMAGE_CREATE_DEST(dst, w, h, t) \
    ((dst) == NULL ? bimageCreate((w), (h), (t)) \
        : ((dst)->width >= (w) && (dst)->height >= (h) && (dst)->type == (t) ? dst : NULL))

#ifdef __cplusplus
}
#endif

#endif // __BIMAGE_HEADER_GUARD
