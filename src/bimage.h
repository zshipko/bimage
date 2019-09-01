#ifndef __BIMAGE_HEADER_GUARD
#define __BIMAGE_HEADER_GUARD

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
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
#define STBI_NO_SIMD
#endif
#else
#ifndef BIMAGE_NO_INTRIN
#define BIMAGE_NO_INTRIN
#endif
#define STBI_NO_SIMD
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
#define bimageTypeChannels(t) ((t)&0x00FF)
#define bimageTypeDepth(t) ((t)&0xFF00)
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
  BIMAGE_C32 = 0x0500,
  BIMAGE_F64 = 0x0600,
} BIMAGE_DEPTH;

typedef struct bimage {
  uint32_t width, height;
  BIMAGE_TYPE type;
  void *data;
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
} bimagePixel;

#ifndef bAlloc
#define bAlloc(n) calloc(n, 1)
#endif

#ifndef bFree
#define bFree(x)                                                               \
  if (x)                                                                       \
  free(x)
#endif

#define bimageSize(w, h, t)                                                    \
  ((int64_t)(w) * (int64_t)(h) * (int64_t)bimageTypeChannels(t))
#define bimageTotalSize(w, h, t)                                               \
  ((int64_t)(w) * (int64_t)(h) *                                               \
   (int64_t)bimageDepthSize(bimageTypeDepth(t)) *                              \
   (int64_t)bimageTypeChannels(t))
#define bimageIndex(im, x, y)                                                  \
  ((y)*bimageTypeChannels((im)->type) * (im)->width +                          \
   (x)*bimageTypeChannels((im)->type))
#define bimageAt(im, index, t) (((t *)(im)->data)[index])
#define bimageIter(im, x, y, _x, _y, _w, _h, sx, sy)                           \
  int32_t x, y;                                                                \
  for (y = _y; y < im->height && y < _y + _h; y += sy)                         \
    for (x = _x; x < im->width && x < _x + _w; x += sx)
#define bimageIterAll(im, x, y)                                                \
  int32_t x, y;                                                                \
  for (y = 0; y < im->height; y++)                                             \
    for (x = 0; x < im->width; x++)

#define bimageBoundsCheck(im, x, y)                                            \
  (bimageIsValid(im) && (im)->width > (x) && (im)->height > (y) && (x) >= 0 && \
   (y) >= 0)

typedef float (*bimagePixelOp)(float, float);
typedef void (*bimageOp)(bimage **dst, bimage *, bimage *, bimagePixelOp);

/* Get the size of a single channel of a single pixel  in bytes */
size_t bimageDepthSize(BIMAGE_DEPTH d);

/* BIMAGE */

/* Initialize an existing pixel */
BIMAGE_STATUS
bimagePixelInit(bimagePixel *px, float r, float g, float b, float a);

/* Create a new pixel */
bimagePixel bimagePixelCreate(float r, float g, float b, float a);

/* Zero an existing pixel */
BIMAGE_STATUS
bimagePixelZero(bimagePixel *px);

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

bimagePixel bimagePixelEq(bimagePixel p, bimagePixel q);

bimagePixel bimagePixelGt(bimagePixel p, bimagePixel q);

bimagePixel bimagePixelLt(bimagePixel p, bimagePixel q);

bool bimagePixelIsTrue(bimagePixel *p);

bool bimagePixelIsFalse(bimagePixel *p);

bool bimagePixelIsEq(bimagePixel p, bimagePixel q);

bool bimagePixelIsGt(bimagePixel p, bimagePixel q);

bool bimagePixelIsLt(bimagePixel p, bimagePixel q);

bimagePixel bimagePixelRandom();

bool bimageIsValid(bimage *im);

double bimageTypeMax(BIMAGE_TYPE t);

bimage *bimageCreateWithData(uint32_t width, uint32_t height, BIMAGE_TYPE t,
                             void *data, bool owner, bool ondisk);

bimage *bimageCreate(uint32_t width, uint32_t height, BIMAGE_TYPE t);

bimage *bimageClone(bimage *image);

bimage *bimageCreateOnDiskFd(int fd, uint32_t width, uint32_t height,
                             BIMAGE_TYPE t);

bimage *bimageCreateOnDisk(const char *filename, uint32_t width,
                           uint32_t height, BIMAGE_TYPE t);

void bimageRelease(bimage *im);

void bimageDestroy(bimage **im);

BIMAGE_STATUS
bimageMapToDisk(const char *filename, bimage **im);

bimage *bimageConsume(bimage **dst, bimage *src);

void *bimageDataOffs(bimage *im, uint32_t x, uint32_t y);

BIMAGE_STATUS
bimageGetPixelUnsafe(bimage *im, uint32_t x, uint32_t y, bimagePixel *p);

BIMAGE_STATUS
bimageSetPixelUnsafe(bimage *im, uint32_t x, uint32_t y, bimagePixel p);

BIMAGE_STATUS
bimageGetPixel(bimage *im, uint32_t x, uint32_t y, bimagePixel *p);

BIMAGE_STATUS
bimageSetPixel(bimage *im, uint32_t x, uint32_t y, bimagePixel p);

bimage *bimageCrop(bimage *dst, bimage *im, uint32_t x, uint32_t y, uint32_t w,
                   uint32_t h);

void bimageCopyTo(bimage *dst, bimage *src, uint32_t x, uint32_t y);

bimage *bimageConvertDepth(bimage *dst, bimage *im, BIMAGE_DEPTH depth);

bimage *bimageConvertChannels(bimage *dst, bimage *im, BIMAGE_CHANNEL channels);

void bimageAdjustGamma(bimage *im, float gamma);

bimagePixel bimageAverageInRect(bimage *im, uint32_t x, uint32_t y,
                                uint32_t width, uint32_t height);

bimage *bimageRandom(bimage *dst, uint32_t width, uint32_t height,
                     BIMAGE_TYPE t);

#ifndef BIMAGE_NO_PTHREAD
typedef bool (*bimageParallelFn)(uint32_t, uint32_t, bimagePixel *, void *);

BIMAGE_STATUS
bimageEachPixel(bimage *im, bimageParallelFn fn, int nthreads, void *);

BIMAGE_STATUS
bimageEachPixel2(bimage *dst, bimage *im, bimageParallelFn fn, int nthreads,
                 void *userdata);
#endif // BIMAGE_NO_PTHREAD

bimage *bimageGetChannel(bimage *dest, bimage *im, int c);

BIMAGE_STATUS
bimageSetChannel(bimage *dest, bimage *im, int c);

bimage **bimageSplitChannels(bimage *im, int *num);

bimage *bimageJoinChannels(bimage *dest, bimage **channels, int num);

/* TIFF */

#ifndef BIMAGE_NO_TIFF

bimage *bimageOpenTIFF(const char *filename);

BIMAGE_STATUS
bimageSaveTIFF(bimage *im, const char *filename);

#endif // BIMAGE_NO_TIFF

/* IO */

bimage *bimageOpen(const char *filename);

bimage *bimageOpen16(const char *filename);

bimage *bimageOpenFloat(const char *filename);

bimage *bimageRead(const unsigned char *buffer, size_t len);

bimage *bimageRead16(const unsigned char *buffer, size_t len);

BIMAGE_STATUS
bimageSave(bimage *im, const char *filename);

BIMAGE_STATUS
bimageSaveJPG(bimage *im, const char *filename, int quality);

/* RESIZE */

bimage *bimageResize(bimage *dst, bimage *im, int32_t width, int32_t height);

/* FILTER */

bool bimageAny(bimage *im, bool (*fn)(bimagePixel *));

bool bimageAll(bimage *im, bool (*fn)(bimagePixel *));

BIMAGE_STATUS
bimageAdd(bimage *dst, bimage *b);

BIMAGE_STATUS
bimageSub(bimage *dst, bimage *b);

BIMAGE_STATUS
bimageMul(bimage *dst, bimage *b);

BIMAGE_STATUS
bimageDiv(bimage *dst, bimage *b);

bimage *bimageEq(bimage *dst, bimage *a, bimage *b);

bimage *bimageGt(bimage *dst, bimage *a, bimage *b);

bimage *bimageLt(bimage *dst, bimage *a, bimage *b);

/* Convert an image to color */
bimage *bimageColor(bimage *dst, bimage *im, BIMAGE_CHANNEL c);

/* Convert an image to grayscale */
bimage *bimageGrayscale(bimage *dst, bimage *im, BIMAGE_CHANNEL c);

/* Invert the color of an image */
bimage *bimageInvert(bimage *dst, bimage *im);

/* Rotate an image by `deg` degrees */
bimage *bimageRotate(bimage *dst, bimage *im, float deg);

/* Perform convolution over `im` using the square matrix `K` with side-length
 * `Ks` */
bimage *bimageFilter(bimage *dst, bimage *im, float *K, int Ks, float divisor,
                     float offset);

bimage *bimageSobelX(bimage *dst, bimage *src);

bimage *bimageSobelY(bimage *dst, bimage *src);

bimage *bimageSobel(bimage *dst, bimage *src);

bimage *bimagePrewittX(bimage *dst, bimage *src);

bimage *bimagePrewittY(bimage *dst, bimage *src);

bimage *bimagePrewitt(bimage *dst, bimage *src);

bimage *bimageOutline(bimage *dst, bimage *src);

bimage *bimageSharpen(bimage *dst, bimage *src);

bimage *bimageBlur(bimage *dst, bimage *src);

bimage *bimageGaussianBlur(bimage *dst, bimage *src);

/* Perform FFT of a BIMAGE_F32 image */
bimage *bimageFFT(bimage *dst, bimage *src);

/* Perform inverse FFT of a BIMAGE_C32 image */
bimage *bimageIFFT(bimage *dst, bimage *src);

#ifndef BIMAGE_NO_CCV

bimage *bimageCanny(bimage *dst, bimage *src, int size, double low_thresh,
                    double high_thresh);

#endif

/* HASH */

uint64_t bimageHash(bimage *im);

void bimageHashString(char dst[9], uint64_t hash);

int bimageHashDiff(uint64_t a, uint64_t b);

#define BIMAGE_CREATE_DEST(dst, w, h, t)                                       \
  ((dst) == NULL                                                               \
       ? bimageCreate((w), (h), (t))                                           \
       : ((dst)->width >= (w) && (dst)->height >= (h) && (dst)->type == (t)    \
              ? dst                                                            \
              : bimageConsume(&(dst), bimageCreate((w), (h), (t)))))

#define BIMAGE_PIXEL_INIT bimagePixelCreate(0, 0, 0, 0)

#define BIMAGE_PIXEL_IS(px, r, g, b, a)                                        \
  (((px).data.f[0] == r) && ((px).data.f[1] == g) && ((px).data.f[2] == b) &&  \
   ((px).data.f[3] == a))

#define BIMAGE_PIXEL_IS_ALL(px, v)                                             \
  (((px).data.f[0] == v) && ((px).data.f[1] == v) && ((px).data.f[2] == v) &&  \
   ((px).data.f[3] == v))

#define BIMAGE_PIXEL_IS_ALL3(px, v)                                            \
  (((px).data.f[0] == v) && ((px).data.f[1] == v) && ((px).data.f[2] == v))

#define BIMAGE_PIXEL_IS_TRUE(px)                                               \
  (((px).data.f[0] == 1) && ((px).data.f[1] == 1) && ((px).data.f[2] == 1) &&  \
   ((px).data.f[3] == 1))

#define BIMAGE_PIXEL_IS_FALSE(px)                                              \
  (((px).data.f[0] == 0) && ((px).data.f[1] == 0) && ((px).data.f[2] == 0) &&  \
   ((px).data.f[3] == 0))

#define BIMAGE_PIXEL_IS_BOOL(px)                                               \
  (((px).data.f[0] == 1 || (px).data.f[0] == 0) &&                             \
   ((px).data.f[1] == 1 || (px).data.f[1] == 0) &&                             \
   ((px).data.f[2] == 1 || (px).data.f[2] == 0) &&                             \
   ((px).data.f[3] == 1 || (px).data.f[3] == 0))

extern int BIMAGE_NUM_CPU;

#ifdef __cplusplus
}
#endif

#endif // __BIMAGE_HEADER_GUARD
