#include "bimage.h"

#include <string.h>

/* PIXEL */

BIMAGE_STATUS
bimagePixelInit(bimagePixel *px, float r, float g, float b, float a) {
  if (!px) {
    return BIMAGE_ERR;
  }

#ifdef BIMAGE_SSE
  px->data.m = _mm_set_ps(a, b, g, r);
#else
  px->data.f[0] = r;
  px->data.f[1] = g;
  px->data.f[2] = b;
  px->data.f[3] = a;
#endif

  return BIMAGE_OK;
}

bimagePixel bimagePixelCreate(float r, float g, float b, float a) {
  bimagePixel px;
  bimagePixelInit(&px, r, g, b, a);
  return px;
}

bimagePixel bimagePixelCopy(bimagePixel *src) {
  bimagePixel px;
  bimagePixelInit(&px, src->data.f[0], src->data.f[1], src->data.f[2],
                  src->data.f[3]);
  return px;
}

BIMAGE_STATUS
bimagePixelZero(bimagePixel *px) { return bimagePixelInit(px, 0, 0, 0, 0); }

BIMAGE_STATUS
bimagePixelClamp(bimagePixel *px) {
  int i;
  for (i = 0; i < 4; i++) {
    px->data.f[i] =
        px->data.f[i] < 0.0 ? 0.0 : px->data.f[i] > 1.0 ? 1.0 : px->data.f[i];
  }

  return BIMAGE_OK;
}

#if !defined(BIMAGE_SSE)
#define PIXEL_OP(name, op)                                                     \
  BIMAGE_STATUS                                                                \
  bimagePixel##name(bimagePixel *a, bimagePixel b) {                           \
    if (!a) {                                                                  \
      return BIMAGE_ERR;                                                       \
    }                                                                          \
    int i;                                                                     \
    for (i = 0; i < 4; i++) {                                                  \
      a->data.f[i] = a->data.f[i] op b.data.f[i];                              \
    }                                                                          \
    bimagePixelClamp(a);                                                       \
    return BIMAGE_OK;                                                          \
  }
#else
#define PIXEL_OP(name, op)                                                     \
  BIMAGE_STATUS                                                                \
  bimagePixel##name(bimagePixel *a, bimagePixel b) {                           \
    if (!a) {                                                                  \
      return BIMAGE_ERR;                                                       \
    }                                                                          \
    a->data.m = a->data.m op b.data.m;                                         \
    bimagePixelClamp(a);                                                       \
    return BIMAGE_OK;                                                          \
  }
#endif

#define PIXEL_COMPARE_OP(name, op)                                             \
  bimagePixel bimagePixel##name(bimagePixel a, bimagePixel b) {                \
    int i;                                                                     \
    bimagePixel c;                                                             \
    for (i = 0; i < 4; i++) {                                                  \
      c.data.f[i] = (float)(a.data.f[i] op b.data.f[i]);                       \
    }                                                                          \
    return c;                                                                  \
  }                                                                            \
                                                                               \
  bool bimagePixelIs##name(bimagePixel a, bimagePixel b) {                     \
    return BIMAGE_PIXEL_IS_TRUE(bimagePixel##name(a, b));                      \
  }

PIXEL_OP(Add, +)
PIXEL_OP(Sub, -)
PIXEL_OP(Mul, *)
PIXEL_OP(Div, /)
PIXEL_COMPARE_OP(Eq, ==)
PIXEL_COMPARE_OP(Gt, >)
PIXEL_COMPARE_OP(Lt, <)

bool bimagePixelIsTrue(bimagePixel *p) { return BIMAGE_PIXEL_IS_TRUE(*p); }

bool bimagePixelIsFalse(bimagePixel *p) { return BIMAGE_PIXEL_IS_FALSE(*p); }

bimagePixel bimagePixelRandom() {
  return bimagePixelCreate(((float)BIMAGE_RAND_RANGE(255)) / 255.0,
                           ((float)BIMAGE_RAND_RANGE(255)) / 255.0,
                           ((float)BIMAGE_RAND_RANGE(255)) / 255.0,
                           ((float)BIMAGE_RAND_RANGE(255)) / 255.0);
}
