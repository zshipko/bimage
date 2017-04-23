#include "bimage.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

bimage*
bimageResize(bimage** dst, bimage* im, int32_t w, int32_t h)
{
    // Scale if one dimension is <= 0,
    // but fail if both `w` and `h`are <= 0
    if (w <= 0 && h <= 0){
        return NULL;
    } else if (w <= 0){
        w = im->width * (h / im->height);
    } else if (h <= 0){
        h = im->height * (w / im->width);
    }

    int dt;

    switch (bimageTypeDepth(im->type)){
    case BIMAGE_U8:
        dt = 0;
        break;
    case BIMAGE_U16:
        dt = 1;
        break;
    case BIMAGE_U32:
        dt = 2;
        break;
    default:
        return NULL;
    }

    bimage *im2 = BIMAGE_CREATE_DEST(dst, w, h, im->type);
    if (!im2){
        return NULL;
    }

    int channels =  bimageTypeChannels(im->type);
    if (stbir_resize(
            im->data, im->width, im->height, 0,
            im2->data, im2->width, im2->height, 0,
            dt, channels, channels > 3 ? 3 : -1, 0, 1, 1,
            STBIR_FILTER_DEFAULT, STBIR_FILTER_DEFAULT,
            STBIR_COLORSPACE_LINEAR, NULL) == 0){
        if (!dst){
            bimageRelease(im2);
        }
        return NULL;
    }

    BIMAGE_RETURN_DEST(dst, im2);
}


