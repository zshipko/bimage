#define _DEFAULT_SOURCE

#include "bimage.h"

#define STBI_NO_FAILURE_STRING
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include <string.h>

#define max(a, b) a>b?a:b
#define min(a, b) a<b?a:b

bimage* bimageOpen(const char *filename)
{
    int w, h, c;
    uint8_t* data = stbi_load(filename, &w, &h, &c, 0);

    if (!data){
#ifdef BIMAGE_NO_TIFF
        return NULL;
#else
        return bimageOpenTIFF(filename);
#endif
    }


    return bimageCreateWithData(w, h, BIMAGE_U8 | c, data, true, false);
}

bimage*
bimageRead(const char *buffer, size_t len)
{
    int w, h, c;
    uint8_t* data = stbi_load_from_memory(buffer, len, &w, &h, &c, 0);
    if (!data){
        return NULL;
    }

    return bimageCreateWithData(w, h, BIMAGE_U8 | c, data, true, false);
}

bimage*
bimageRead16(const char *buffer, size_t len)
{
    int w, h, c;
    uint16_t* data = stbi_load_16_from_memory(buffer, len, &w, &h, &c, 0);
    if (!data){
        return NULL;
    }

    return bimageCreateWithData(w, h, BIMAGE_U16 | c, data, true, false);
}

bimage *bimageOpen16(const char *filename)
{
    int w, h, c;
    uint16_t* data = stbi_load_16(filename, &w, &h, &c, 0);
    if (!data){
#ifdef BIMAGE_NO_TIFF
        return NULL;
#else
        // Make sure TIFF is the correct depth
        bimage *im = bimageOpenTIFF(filename);
        if (!im){
            return NULL;
        }
        if (bimageTypeDepth(im->type) != BIMAGE_U16){
            bimageRelease(im);
            return NULL;
        }
        return im;
#endif
    }

    return bimageCreateWithData(w, h, BIMAGE_U16 | c, data, true, false);
}

bimage*
bimageOpenFloat(const char *filename)
{
    int w, h, c;
    float* data = stbi_loadf(filename, &w, &h, &c, 0);
    if (!data){
#ifdef BIMAGE_NO_TIFF
        return NULL;
#else
        // Make sure TIFF is the correct depth
        bimage *im = bimageOpenTIFF(filename);
        if (!im){
            return NULL;
        }

        if (bimageTypeDepth(im->type) != BIMAGE_F32){
            bimageRelease(im);
            return NULL;
        }
        return im;
#endif
    }

    return bimageCreateWithData(w, h, BIMAGE_F32 | c, data, true, false);
}


const char *get_ext(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot+1;
}

BIMAGE_STATUS
bimageSaveJPG(bimage *im, const char *filename, int quality)
{
    if (bimageTypeDepth(im->type) != BIMAGE_U8){
        return BIMAGE_ERR_INVALID_FORMAT;
    }

    return stbi_write_jpg(filename,
                im->width,
                im->height,
                bimageTypeChannels(im->type),
                im->data, quality) == 0 ? BIMAGE_ERR : BIMAGE_OK;
}

BIMAGE_STATUS
bimageSave(bimage *im, const char *filename)
{
    const char *x = get_ext(filename);
    if (strncasecmp(x, "png", 3) == 0 && bimageTypeDepth(im->type) == BIMAGE_U8){
		return stbi_write_png(filename,
                im->width,
                im->height,
                bimageTypeChannels(im->type),
                im->data, im->width * bimageTypeChannels(im->type)) == 0 ? BIMAGE_ERR : BIMAGE_OK;
    } else if (strncasecmp(x, "hdr", 3) == 0 && bimageTypeDepth(im->type) == BIMAGE_F32){
        return stbi_write_hdr(filename,
                im->width,
                im->height,
                bimageTypeChannels(im->type),
                im->data) == 0 ? BIMAGE_ERR : BIMAGE_OK;
    } else if ((strncasecmp(x, "jpg", 3) == 0 ||
                strncasecmp(x, "jpeg", 3) == 0) &&
                bimageTypeDepth(im->type) == BIMAGE_U8){
        return bimageSaveJPG(im, filename, 95);
#ifdef BIMAGE_NO_TIFF
    }
#else
    } else if (strncasecmp(x, "tif", 3) == 0 || strncasecmp(x, "tiff", 4) == 0){
		return bimageSaveTIFF(im, filename);
	}
#endif

	return BIMAGE_ERR_INVALID_FORMAT;
}



