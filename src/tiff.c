#include "bimage.h"

#include <stdlib.h>
#include <string.h>
#include <tiffio.h>

static void
tiffErrorHandler (const char* module, const char* fmt, va_list ap)
{
    // ignore
}

static bool tiffInitialized = false;

static void
tiffInit()
{
    TIFFSetWarningHandler(tiffErrorHandler);
    TIFFSetErrorHandler(tiffErrorHandler);
    tiffInitialized = true;
}


BIMAGE_STATUS
bimageSaveTIFF(bimage *im, const char *filename)
{
    if (!tiffInitialized){
        tiffInit();
    }

    TIFF *tif = TIFFOpen(filename, "w");
    if (!tif){
        return BIMAGE_ERR;
    }

    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, im->width);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, im->height);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bimageTypeDepth(im->type));
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, bimageTypeChannels(im->type));

    uint32_t j;
    for (j = 0; j < im->height; j++){
        if (TIFFWriteScanline(tif, im->data + ((bimageTypeDepth(im->type)/8) * bimageTypeChannels(im->type) * im->width * j), j, 0) < 0){
            TIFFClose(tif);
            remove(filename);
            return BIMAGE_ERR;
        }
    }

    TIFFClose(tif);
    return BIMAGE_OK;
}

bimage*
bimageOpenTIFF(const char *filename)
{
    if (!tiffInitialized){
        tiffInit();
    }

    TIFF *tif = TIFFOpen(filename, "r");
    bimage *im = NULL;

    if (!tif){
        return NULL;
    }

    uint32_t w, h;
    uint16_t depth, channels;

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
    TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &depth);
    TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &channels);

    BIMAGE_TYPE t;
    if (bimageMakeType(&t, channels, depth) == BIMAGE_OK){
        im = bimageCreate(w, h,t);
    } else {
        goto done;
    }

    tstrip_t strip;
    uint8_t *buf = bAlloc(TIFFStripSize(tif));
    if (!buf){
        goto error0;
    }

    for(strip = 0; strip < TIFFNumberOfStrips(tif); strip++){
        tsize_t n = TIFFReadEncodedStrip(tif, strip, buf, -1);
        if (n < 0){
            goto error1;
        }

        memcpy(im->data + TIFFStripSize(tif) * strip, buf, n);
    }

    bFree(buf);
done:
    TIFFClose(tif);
    return im;

error1:
    bFree(buf);
error0:
    if (im) bimageDestroy(&im);
    goto done;
}


