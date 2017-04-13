#include "bimage.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

bimage*
bimageResize(bimage* im, int32_t w, int32_t h)
{
    int dt;

    switch (bimageTypeSize(im->type)){
    case 8:
        dt = 0;
        break;
    case 16:
        dt = 1;
        break;
    case 32:
        dt = 2;
        break;
    }

    bimage *dst = bimageCreate(w, h, im->type);
    int channels =  bimageTypeChannels(im->type);
    if (stbir_resize(
            im->data, im->width, im->height, 0,
            dst->data, dst->width, dst->height, 0,
            dt, channels, channels > 3 ? 3 : -1, 0, 1, 1,
            STBIR_FILTER_DEFAULT, STBIR_FILTER_DEFAULT,
            STBIR_COLORSPACE_LINEAR, NULL) == 0){
        bimageRelease(dst);
        return NULL;
    }

    return dst;
}