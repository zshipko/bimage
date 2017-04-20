#include "bimage.h"

#include <string.h>

/* PIXEL */

bpixel
bpixelCreate (float r, float g, float b, float a, BIMAGE_DEPTH depth)
{
    bpixel px;
    bzero(&px, sizeof(bpixel));
    px.depth = depth;
    px.data[0] = r;
    px.data[1] = g;
    px.data[2] = b;
    px.data[3] = a;
    return px;
}

bpixel
bpixelZero(BIMAGE_DEPTH depth)
{
    return bpixelCreate(0, 0, 0, 0, depth);
}

BIMAGE_STATUS
bpixelConvertDepth (bpixel a, BIMAGE_DEPTH depth, bpixel *dst)
{
    if (!dst){
        return BIMAGE_ERR;
    }

    int i;
    switch (a.depth) {
    case BIMAGE_U8:
        switch (depth) {
        case BIMAGE_U8:
            *dst = a;
            break;
        case BIMAGE_U16:
            for (i = 0; i < 4; i++){
                (*dst).data[i] = (uint32_t)a.data[i] << 8;
            }
            break;
        case BIMAGE_U32:
            for (i = 0; i < 4; i++){
                (*dst).data[i] = (uint32_t)a.data[i] << 24;
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
                (*dst).data[i] = (uint32_t)a.data[i] >> 8;
            }
            break;
        case BIMAGE_U16:
            *dst = a;
            break;
        case BIMAGE_U32:
            for (i = 0; i < 4; i++){
                (*dst).data[i] = (uint32_t)a.data[i] << 16;
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
                (*dst).data[i] = (uint32_t)a.data[i] >> 24;
            }
            break;
        case BIMAGE_U16:
            for (i = 0; i < 4; i++){
                (*dst).data[i] = (uint32_t)a.data[i] >> 16;
            }
            break;
        case BIMAGE_U32:
            *dst = a;
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

