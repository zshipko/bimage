#include "bimage.h"

#include <string.h>

/* PIXEL */

BIMAGE_STATUS
bimagePixelInit(bimagePixel *px, float r, float g, float b, float a, BIMAGE_DEPTH depth){
    if (!px){
        return BIMAGE_ERR;
    }

    px->depth = depth;
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

bimagePixel
bimagePixelCreate (float r, float g, float b, float a, BIMAGE_DEPTH depth)
{
    bimagePixel px;
    bimagePixelInit(&px, r, g, b, a, depth);
    return px;
}

BIMAGE_STATUS
bimagePixelZero(bimagePixel *px, BIMAGE_DEPTH depth)
{
    return bimagePixelInit(px, 0, 0, 0, 0, depth);
}

BIMAGE_STATUS
bimagePixelConvertDepth (bimagePixel *dst, bimagePixel src, BIMAGE_DEPTH depth)
{
    // Same depth
    if (src.depth == depth || src.depth == BIMAGE_UNKNOWN){
        *dst = src;
        goto ok;
    }

#ifndef BIMAGE_SSE
    int i;
#endif

    // Conversion to and from F32 is the same for every type
    if (depth == BIMAGE_F32 || depth == BIMAGE_F64 || depth == BIMAGE_C32){
        float mx = bimageTypeMax(src.depth);
        (*dst).depth = depth;
#ifdef BIMAGE_SSE
        (*dst).data.m = src.data.m/_mm_load_ps1(&mx);
#else
        for(i = 0; i < 4; i++){
            (*dst).data.f[i] = src.data.f[i]/mx;
        }
#endif
        goto ok;
    } else if (src.depth == BIMAGE_F32 || src.depth == BIMAGE_F64 || src.depth == BIMAGE_C32){
        float mx = bimageTypeMax(depth);
        (*dst).depth = depth;
#ifdef BIMAGE_SSE
        (*dst).data.m = src.data.m * _mm_load_ps1(&mx);
#else
        for(i = 0; i < 4; i++){
            (*dst).data.f[i] = src.data.f[i] * mx;
        }
#endif
        goto ok;

    }

#ifdef BIMAGE_SSE
    __m128i x = _mm_castps_si128(src.data.m);
#endif
    switch (src.depth) {
    case BIMAGE_U8:
        switch (depth) {
        case BIMAGE_U16: // Convert to U16 from U8
#ifdef BIMAGE_SSE
            (*dst).data.m = _mm_castsi128_ps (_mm_slli_si128(x, 8));
#else
            for (i = 0; i < 4; i++){
                (*dst).data.f[i] = (uint32_t)src.data.f[i] << 8;
            }
#endif
            break;
        case BIMAGE_U32: // Convert to U32 from U8
#ifdef BIMAGE_SSE
            (*dst).data.m = _mm_castsi128_ps(_mm_slli_si128(x, 24));
#else
            for (i = 0; i < 4; i++){
                (*dst).data.f[i] = (uint32_t)src.data.f[i] << 24;
            }
#endif
            break;
        default:
            return BIMAGE_ERR;
        }
        break;
    case BIMAGE_U16:
       switch (depth) {
        case BIMAGE_U8:  // Convert to U8 from U16
#ifdef BIMAGE_SSE
            (*dst).data.m = _mm_castsi128_ps(_mm_srli_si128(x, 8));
#else
            for (i = 0; i < 4; i++){
                (*dst).data.f[i] = (uint32_t)src.data.f[i] >> 8;
            }
#endif
            break;
        case BIMAGE_U32: // Convert to U32 from U16
#ifdef BIMAGE_SSE
            (*dst).data.m = _mm_castsi128_ps(_mm_slli_si128(x,  16));
#else
            for (i = 0; i < 4; i++){
                (*dst).data.f[i] = (uint32_t)src.data.f[i] << 16;
            }
#endif
            break;
        default:
            return BIMAGE_ERR;
        }
       break;
    case BIMAGE_U32:
       switch (depth) {
        case BIMAGE_U8:  // Convert to U8 from U32
#ifdef BIMAGE_SSE
            (*dst).data.m = _mm_castsi128_ps(_mm_srli_si128(x, 24));
#else
            for (i = 0; i < 4; i++){
                (*dst).data.f[i] = (uint32_t)src.data.f[i] >> 24;
            }
#endif
            break;
        case BIMAGE_U16: // Convert to U16 from U32
#ifdef BIMAGE_SSE
            (*dst).data.m = _mm_castsi128_ps(_mm_srli_si128(x, 16));
#else
            for (i = 0; i < 4; i++){
                (*dst).data.f[i] = (uint32_t)src.data.f[i] >> 16;
            }
#endif
            break;
        default:
            return BIMAGE_ERR;
        }
       break;
    default:
       return BIMAGE_ERR;
    }

ok:
    (*dst).depth = depth;
    return BIMAGE_OK;
}

BIMAGE_STATUS
bimagePixelClamp(bimagePixel *px)
{
    int i;
    BIMAGE_TYPE t = px->depth | 1;
    float mx = (float)bimageTypeMax(t);
    for(i = 0; i < 4; i++){
        px->data.f[i] = px->data.f[i] < 0 ? 0 : (px->data.f[i] > mx ? mx : px->data.f[i]);
    }

    return BIMAGE_OK;
}

#if !defined(BIMAGE_SSE)
#define PIXEL_OP(name, op) \
BIMAGE_STATUS \
bimagePixel##name(bimagePixel *a, bimagePixel b) \
{ \
    if (!a){ \
        return BIMAGE_ERR; \
    } \
    int i; \
    bimagePixel c; \
    bimagePixelConvertDepth(&c, b, a->depth); \
    for (i = 0; i < 4; i++){ \
        a->data.f[i] = a->data.f[i] op c.data.f[i]; \
    } \
    bimagePixelClamp(a); \
    return BIMAGE_OK; \
}
#else
#define PIXEL_OP(name, op) \
BIMAGE_STATUS \
bimagePixel##name(bimagePixel *a, bimagePixel b) \
{ \
    if (!a){ \
        return BIMAGE_ERR; \
    } \
    bimagePixel c; \
    bimagePixelConvertDepth(&c, b, a->depth); \
    a->data.m = a->data.m op c.data.m;\
    bimagePixelClamp(a); \
    return BIMAGE_OK; \
}
#endif

#define PIXEL_COMPARE_OP(name, op) \
bimagePixel \
bimagePixel##name(bimagePixel a, bimagePixel b) \
{ \
    int i; \
    bimagePixel c; \
    bimagePixelConvertDepth(&c, b, a.depth); \
    for (i = 0; i < 4; i++){ \
        c.data.f[i] = (a.data.f[i] op c.data.f[i]); \
    } \
    return c; \
} \
\
bool \
bimagePixelIs##name(bimagePixel a, bimagePixel b) \
{ \
    return BIMAGE_PIXEL_IS_TRUE(bimagePixel##name(a, b)); \
}


PIXEL_OP(Add, +)
PIXEL_OP(Sub, -)
PIXEL_OP(Mul, *)
PIXEL_OP(Div, /)
PIXEL_COMPARE_OP(Eq, ==)
PIXEL_COMPARE_OP(Gt, >)
PIXEL_COMPARE_OP(Lt, <)

bool
bimagePixelIsTrue(bimagePixel *p)
{
    return BIMAGE_PIXEL_IS_TRUE(*p);
}

bool
bimagePixelIsFalse(bimagePixel *p)
{
    return BIMAGE_PIXEL_IS_FALSE(*p);
}

bimagePixel
bimagePixelRandom(BIMAGE_DEPTH t)
{
    if (t == BIMAGE_F32){
        return bimagePixelCreate(
            (float)BIMAGE_RAND_RANGE(255) / 255.0,
            (float)BIMAGE_RAND_RANGE(255) / 255.0,
            (float)BIMAGE_RAND_RANGE(255) / 255.0,
            (float)BIMAGE_RAND_RANGE(255) / 255.0, t);
    }

    return bimagePixelCreate(
        BIMAGE_RAND_RANGE(bimageDepthMax(t)),
        BIMAGE_RAND_RANGE(bimageDepthMax(t)),
        BIMAGE_RAND_RANGE(bimageDepthMax(t)),
        BIMAGE_RAND_RANGE(bimageDepthMax(t)), t);
}
