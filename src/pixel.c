#include "bimage.h"

#include <string.h>

/* PIXEL */

BIMAGE_STATUS
bpixelInit(bpixel *px, float r, float g, float b, float a, BIMAGE_DEPTH depth){
    if (!px){
        return BIMAGE_ERR;
    }

    px->depth = depth;
    px->data[0] = r;
    px->data[1] = g;
    px->data[2] = b;
    px->data[3] = a;

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
bpixelConvertDepth (bpixel *dst, bpixel *src, BIMAGE_DEPTH depth)
{
    if (!dst){
        return BIMAGE_ERR;
    }

    int i;
    switch (src->depth) {
    case BIMAGE_U8:
        switch (depth) {
        case BIMAGE_U8:
            *dst = *src;
            break;
        case BIMAGE_U16:
            for (i = 0; i < 4; i++){
                (*dst).data[i] = (uint32_t)src->data[i] << 8;
            }
            break;
        case BIMAGE_U32:
            for (i = 0; i < 4; i++){
                (*dst).data[i] = (uint32_t)src->data[i] << 24;
            }
            break;
        default:
            return BIMAGE_ERR;
        }
        break;
    case BIMAGE_U16:
       switch (depth) {
        case BIMAGE_U8:
            for (i = 0; i < 4; i++){
                (*dst).data[i] = (uint32_t)src->data[i] >> 8;
            }
            break;
        case BIMAGE_U16:
            *dst = *src;
            break;
        case BIMAGE_U32:
            for (i = 0; i < 4; i++){
                (*dst).data[i] = (uint32_t)src->data[i] << 16;
            }
            break;
        default:
            return BIMAGE_ERR;
        }
       break;
    case BIMAGE_U32:
       switch (depth) {
        case BIMAGE_U8:
            for (i = 0; i < 4; i++){
                (*dst).data[i] = (uint32_t)src->data[i] >> 24;
            }
            break;
        case BIMAGE_U16:
            for (i = 0; i < 4; i++){
                (*dst).data[i] = (uint32_t)src->data[i] >> 16;
            }
            break;
        case BIMAGE_U32:
            *dst = *src;
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

BIMAGE_STATUS
bpixelClamp(bpixel *px)
{
    int i;
    BIMAGE_TYPE t;

    if (!px || bimageMakeType(1, px->depth, &t) == BIMAGE_ERR){
        return BIMAGE_ERR;
    }

    float mx = (float)bimageTypeMax(t);
    for(i = 0; i < 4; i++){
        px->data[i] = px->data[i] < 0 ? 0 : (px->data[i] > mx ? mx : px->data[i]);
    }

    return BIMAGE_OK;
}

#define PIXEL_OP(name, op) \
BIMAGE_STATUS \
bpixel##name(bpixel *a, bpixel *b) \
{ \
    if (!a || !b){ \
        return BIMAGE_ERR; \
    } \
    int i; \
    bpixel c; \
    if (a->depth != b->depth){ \
        bpixelConvertDepth(&c, b, a->depth); \
        b = &c; \
    } \
    for (i = 0; i < 3; i++){ \
        a->data[i] = a->data[i] op b->data[i]; \
    } \
    bpixelClamp(a); \
    return BIMAGE_OK; \
}

PIXEL_OP(Add, +);
PIXEL_OP(Sub, -);
PIXEL_OP(Mul, *);
PIXEL_OP(Div, /);
