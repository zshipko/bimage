#include "bimage.h"

#include <string.h>

/* PIXEL */

pixel
pixelCreate (float r, float g, float b, float a, int depth)
{
    pixel px;
    bzero(&px, sizeof(pixel));
    px.depth = depth;
    px.data[0] = r;
    px.data[1] = g;
    px.data[2] = b;
    px.data[3] = a;
    return px;
}

BIMAGE_STATUS
pixelConvertDepth (pixel a, uint8_t depth, pixel *dst)
{
    if (!dst){
        return BIMAGE_ERR;
    }

    int i;
    switch (a.depth) {
    case 8:
        switch (depth) {
        case 8:
            *dst = a;
            break;
        case 16:
            for (i = 0; i < 4; i++){
                (*dst).data[i] = (uint32_t)a.data[i] << 8;
            }
            break;
        case 32:
            for (i = 0; i < 4; i++){
                (*dst).data[i] = (uint32_t)a.data[i] << 24;
            }
            break;
        default:
            return BIMAGE_ERR;
        }
        break;
    case 16:
       switch (depth) {
        case 8:
            for (i = 0; i < 4; i++){
                (*dst).data[i] = (uint32_t)a.data[i] >> 8;
            }
            break;
        case 16:
            *dst = a;
            break;
        case 32:
            for (i = 0; i < 4; i++){
                (*dst).data[i] = (uint32_t)a.data[i] << 16;
            }
            break;
        default:
            return BIMAGE_ERR;
        }
       break;
    case 32:
       switch (depth) {
        case 8:
            for (i = 0; i < 4; i++){
                (*dst).data[i] = (uint32_t)a.data[i] >> 24;
            }
            break;
        case 16:
            for (i = 0; i < 4; i++){
                (*dst).data[i] = (uint32_t)a.data[i] >> 16;
            }
            break;
        case 32:
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

