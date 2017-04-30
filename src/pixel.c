#include "bimage.h"

#include <string.h>

/* PIXEL */

BIMAGE_STATUS
bpixelInit(bpixel *px, float r, float g, float b, float a, BIMAGE_DEPTH depth){
    if (!px){
        return BIMAGE_ERR;
    }

    px->depth = depth;
#ifdef BIMAGE_SSE
    px->m = _mm_set_ps(a, b, g, r);
#else
    px->data[0] = r;
    px->data[1] = g;
    px->data[2] = b;
    px->data[3] = a;
#endif

    return BIMAGE_OK;
}

bpixel
bpixelCreate (float r, float g, float b, float a, BIMAGE_DEPTH depth)
{
    bpixel px;
    bpixelInit(&px, r, g, b, a, depth);
    return px;
}

BIMAGE_STATUS
bpixelZero(bpixel *px, BIMAGE_DEPTH depth)
{
    return bpixelInit(px, 0, 0, 0, 0, depth);
}

BIMAGE_STATUS
bpixelConvertDepth (bpixel *dst, bpixel src, BIMAGE_DEPTH depth)
{
    int i;

    // Same depth
    if (src.depth == depth){
        *dst = src;
        goto ok;
    }

    // Conversion to F32 is the same for every type
    if (depth == BIMAGE_F32){
        float mx = bimageTypeMax(src.depth);
        (*dst).depth = BIMAGE_F32;
#ifdef BIMAGE_SSE
        (*dst).m = src.m/_mm_load_ps1(&mx);
#else
        for(i = 0; i < 4; i++){
            (*dst).data[i] = src.data[i]/mx;
        }
#endif
        goto ok;
    }

#ifdef BIMAGE_SSE
    __m128i x = _mm_castps_si128(src.m);
#endif
    switch (src.depth) {
    case BIMAGE_U8:
        switch (depth) {
        case BIMAGE_U16: // Convert to U16 from U8
#ifdef BIMAGE_SSE
            (*dst).m = _mm_castsi128_ps (_mm_slli_si128(x, 8));
#else
            for (i = 0; i < 4; i++){
                (*dst).data[i] = (uint32_t)src.data[i] << 8;
            }
#endif
            break;
        case BIMAGE_U32: // Convert to U32 from U8
#ifdef BIMAGE_SSE
            (*dst).m = _mm_castsi128_ps(_mm_slli_si128(x, 24));
#else
            for (i = 0; i < 4; i++){
                (*dst).data[i] = (uint32_t)src.data[i] << 24;
            }
#endif
            break;
        default:
            return BIMAGE_ERR;
        }
        break;
    case BIMAGE_U16:
       switch (depth) {
#ifdef BIMAGE_SSE
        (*dst).m = _mm_castsi128_ps(_mm_srli_si128(x, 8));
#else
        case BIMAGE_U8:  // Convert to U8 from U16
            for (i = 0; i < 4; i++){
                (*dst).data[i] = (uint32_t)src.data[i] >> 8;
            }
#endif
            break;
        case BIMAGE_U32: // Convert to U32 from U16
#ifdef BIMAGE_SSE
            (*dst).m = _mm_castsi128_ps(_mm_slli_si128(x,  16));
#else
            for (i = 0; i < 4; i++){
                (*dst).data[i] = (uint32_t)src.data[i] << 16;
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
            (*dst).m = _mm_castsi128_ps(_mm_srli_si128(x, 24));
#else
            for (i = 0; i < 4; i++){
                (*dst).data[i] = (uint32_t)src.data[i] >> 24;
            }
#endif
            break;
        case BIMAGE_U16: // Convert to U16 from U32
#ifdef BIMAGE_SSE
            (*dst).m = _mm_castsi128_ps(_mm_srli_si128(x, 16));
#else
            for (i = 0; i < 4; i++){
                (*dst).data[i] = (uint32_t)src.data[i] >> 16;
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
bpixelClamp(bpixel *px)
{
    int i;
    BIMAGE_TYPE t = px->depth | 1;
    float mx = (float)bimageTypeMax(t);
    for(i = 0; i < 4; i++){
        px->data[i] = px->data[i] < 0 ? 0 : (px->data[i] > mx ? mx : px->data[i]);
    }

    return BIMAGE_OK;
}

#if !defined(BIMAGE_SSE) // && !defined(BIMAGE_NEON)
#define PIXEL_OP(name, op) \
BIMAGE_STATUS \
bpixel##name(bpixel *a, bpixel b) \
{ \
    if (!a){ \
        return BIMAGE_ERR; \
    } \
    int i; \
    bpixel c; \
    bpixelConvertDepth(&c, b, a->depth); \
    for (i = 0; i < 3; i++){ \
        a->data[i] = a->data[i] op c.data[i]; \
    } \
    bpixelClamp(a); \
    return BIMAGE_OK; \
}
#else
#define PIXEL_OP(name, op) \
BIMAGE_STATUS \
bpixel##name(bpixel *a, bpixel b) \
{ \
    if (!a){ \
        return BIMAGE_ERR; \
    } \
    int i; \
    bpixel c; \
    bpixelConvertDepth(&c, b, a->depth); \
    a->m = a->m op c.m;\
    bpixelClamp(a); \
    return BIMAGE_OK; \
}
#endif

PIXEL_OP(Add, +);
PIXEL_OP(Sub, -);
PIXEL_OP(Mul, *);
PIXEL_OP(Div, /);
