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
    BIMAGE_TYPE t;
    uint8_t* data = stbi_load(filename, &w, &h, &c, 0);

    if (!data){
        return bimageOpenTIFF(filename);
    }

    if (bimageMakeType(c, 8, &t) == BIMAGE_OK){
        return bimageCreateWithData(w, h, t, data, true, false);
    }

    bFree(data);
    return NULL;
}

bimage *bimageOpen16(const char *filename)
{
    int w, h, c;
    BIMAGE_TYPE t;

    uint16_t* data = stbi_load_16(filename, &w, &h, &c, 0);
    if (!data){
        return bimageOpenTIFF(filename);
    }

    if (bimageMakeType(c, 16, &t) == BIMAGE_OK){
        return bimageCreateWithData(w, h, t, data, true, false);
    }

    bFree(data);
    return NULL;
}


const char *get_ext(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot+1;
}

BIMAGE_STATUS
bimageSave(bimage *im, const char *filename)
{
    const char *x = get_ext(filename);
    if (strncasecmp(x, "png", 3) == 0 && bimageTypeSize(im->type) == 8){
		return stbi_write_png(filename,
                im->width,
                im->height,
                bimageTypeChannels(im->type),
                im->data, im->width * bimageTypeChannels(im->type)) == 0 ? BIMAGE_ERR : BIMAGE_OK;
    } else if (strncasecmp(x, "tif", 3) == 0 || strncasecmp(x, "tiff", 4) == 0){
		return bimageSaveTIFF(im, filename);
	}

	return BIMAGE_ERR;
}


