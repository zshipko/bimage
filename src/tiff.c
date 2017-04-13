#include "bimage.h"

#include <stdlib.h>
#include <string.h>
#include <tiffio.h>

BIMAGE_STATUS
bimageSaveTIFF(bimage *im, const char *filename)
{
    TIFF *tif = TIFFOpen(filename, "w");
    if (!tif){
        return BIMAGE_ERR;
    }

    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, im->width);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, im->height);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bimageTypeSize(im->type));
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, bimageTypeChannels(im->type));

    uint32_t j;
    for (j = 0; j < im->height; j++){
        if (TIFFWriteScanline(tif, im->data + ((bimageTypeSize(im->type)/8) * bimageTypeChannels(im->type) * im->width * j), j, 0) < 0){
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
    TIFF *tif = TIFFOpen(filename, "r");
    bimage *im = NULL;

    if (!tif){
        return NULL;
    }

    uint32_t w, h;
    size_t npix;
    uint32_t* raster;
    uint16_t depth, channels;

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
    TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &depth);
    TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &channels);

    BIMAGE_TYPE t;
    if (bimageMakeType(channels, depth, &t) == BIMAGE_OK){
        im = bimageCreate(w, h,t);
    } else {
        goto done;
    }

    tstrip_t strip;
    uint8_t *buf = bAlloc(TIFFStripSize(tif));
    if (!buf){
        goto error;
    }

    for(strip = 0; strip < TIFFNumberOfStrips(tif); strip++){
        tsize_t n = TIFFReadEncodedStrip(tif, strip, buf, -1);
        if (n < 0){
            goto error;
        }

        memcpy(im->data + TIFFStripSize(tif) * strip, buf, n);
    }

done:
    if (buf) bFree(buf);
    TIFFClose(tif);

    return im;

error:
    if (im) bimageRelease(im);
    im = NULL;
    goto done;
}


