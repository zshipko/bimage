#ifndef __BIMAGE_HEADER_GUARD
#define __BIMAGE_HEADER_GUARD

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#if defined(__has_include) && !defined(BIMAGE_NO_INTRIN)
#if (__has_include(<emmintrin.h>))
#include <emmintrin.h>
#define BIMAGE_SSE
#else
#define BIMAGE_NO_INTRIN
#endif
#else
#ifndef BIMAGE_NO_INTRIN
#define BIMAGE_NO_INTRIN
#endif
#endif

#ifndef BIMAGE_RAND_RANGE
#ifdef __linux__
#define BIMAGE_RAND_RANGE(x) (random() % x)
#else
#define BIMAGE_RAND_RANGE(x) arc4random_uniform(x)
#endif
#endif

typedef enum BIMAGE_STATUS {
    BIMAGE_OK,
    BIMAGE_ERR,
    BIMAGE_ERR_INVALID_FORMAT
} BIMAGE_STATUS;

typedef uint16_t BIMAGE_TYPE;

#define bimageType(d, c) (d) | (c & 0xFF)
#define bimageTypeChannels(t) ((t) & 0x00FF)
#define bimageTypeDepth(t) ((t) & 0xFF00)
#define bimageDepthMax(d) (bimageTypeMax(d | 1))

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

#ifndef bAlloc
#define bAlloc(n) calloc(n, 1)
#endif

#ifndef bFree
#define bFree(x) if(x) free(x)
#endif

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

bimagePixel
bimagePixelEq(bimagePixel p, bimagePixel q);

bimagePixel
bimagePixelGt(bimagePixel p, bimagePixel q);

bimagePixel
bimagePixelLt(bimagePixel p, bimagePixel q);

bimagePixel
bimagePixelRandom(BIMAGE_DEPTH depth);

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

bimage*
bimageConvertDepth(bimage* dst, bimage* im, BIMAGE_DEPTH depth);

bimage*
bimageConvertChannels(bimage* dst, bimage* im, BIMAGE_CHANNEL channels);

void
bimageAdjustGamma(bimage* im, float gamma);

bimagePixel
bimageAverageInRect(bimage* im, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

bimage*
bimageRandom(bimage* dst, uint32_t width, uint32_t height, BIMAGE_TYPE t);

#ifndef BIMAGE_NO_PTHREAD
typedef bool (*bimageParallelFn)(uint32_t, uint32_t, bimagePixel *, void *);

BIMAGE_STATUS
bimageEachPixel(bimage* im, bimageParallelFn fn, int nthreads, void *);

BIMAGE_STATUS
bimageEachPixel2(bimage *dst, bimage *im, bimageParallelFn fn, int nthreads, void *userdata);
#endif // BIMAGE_NO_PTHREAD

/* TIFF */

#ifndef BIMAGE_NO_TIFF

bimage*
bimageOpenTIFF(const char* filename);

BIMAGE_STATUS
bimageSaveTIFF(bimage* im, const char* filename);

#endif // BIMAGE_NO_TIFF

/* IO */

bimage*
bimageOpen(const char* filename);

bimage*
bimageOpen16(const char* filename);

bimage*
bimageOpenFloat(const char* filename);

bimage*
bimageRead(const unsigned char *buffer, size_t len);

bimage*
bimageRead16(const unsigned char *buffer, size_t len);

BIMAGE_STATUS
bimageSave(bimage* im, const char* filename);

BIMAGE_STATUS
bimageSaveJPG(bimage *im, const char *filename, int quality);

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

BIMAGE_STATUS
bimageEq(bimage *dst, bimage *b);

BIMAGE_STATUS
bimageGt(bimage *dst, bimage *b);

BIMAGE_STATUS
bimageLt(bimage *dst, bimage *b);

bimage*
bimageColor(bimage* dst, bimage* im, BIMAGE_CHANNEL c);

bimage*
bimageGrayscale(bimage* dst, bimage* im, BIMAGE_CHANNEL c);

bimage*
bimageInvert(bimage* dst, bimage* im);

bimage*
bimageRotate(bimage* dst, bimage* im, float deg);

bimage*
bimageFilter(bimage* dst, bimage* im, float* K, int Ks, float divisor, float offset);

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

/* HASH */

uint64_t
bimageHash(bimage *im);

void
bimageHashString(char dst[9], uint64_t hash);

int
bimageHashDiff(uint64_t a, uint64_t b);

#define BIMAGE_CREATE_DEST(dst, w, h, t) \
    ((dst) == NULL ? bimageCreate((w), (h), (t)) \
        : ((dst)->width >= (w) && (dst)->height >= (h) && (dst)->type == (t) ? dst : NULL))


#define BIMAGE_PIXEL_INIT bimagePixelCreate(0, 0, 0, 0, BIMAGE_UNKNOWN)

#define BIMAGE_PIXEL_IS(px, r, g, b, a) \
    (((px).data.f[0] == r) && \
     ((px).data.f[1] == g) && \
     ((px).data.f[2] == b) && \
     ((px).data.f[3] == a))

#define BIMAGE_PIXEL_IS_ALL(px, v) \
    (((px).data.f[0] == v) && \
     ((px).data.f[1] == v) && \
     ((px).data.f[2] == v) && \
     ((px).data.f[3] == v))

#define BIMAGE_PIXEL_IS_TRUE(px) BIMAGE_PIXEL_IS_ALL(px, 1)
#define BIMAGE_PIXEL_IS_FALSE(px) BIMAGE_PIXEL_IS_ALL(px, 0)
#define BIMAGE_PIXEL_IS_BOOL(px) \
    (((px).data.f[0] == 1 || (px).data.f[0] == 0) && \
     ((px).data.f[1] == 1 || (px).data.f[1] == 0) && \
     ((px).data.f[2] == 1 || (px).data.f[2] == 0) && \
     ((px).data.f[3] == 1 || (px).data.f[3] == 0))

extern int BIMAGE_NUM_CPU;

#ifdef __cplusplus
}
#endif

#endif // __BIMAGE_HEADER_GUARD
