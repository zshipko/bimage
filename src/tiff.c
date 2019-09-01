#include "bimage.h"

#include <stdlib.h>
#include <string.h>
#include <tiffio.h>

static void tiffErrorHandler(const char *module, const char *fmt, va_list ap) {
  // ignore
}

static bool tiffInitialized = false;

static void tiffInit() {
  TIFFSetWarningHandler(tiffErrorHandler);
  TIFFSetErrorHandler(tiffErrorHandler);
  tiffInitialized = true;
}

BIMAGE_STATUS
bimageSaveTIFF(bimage *im, const char *filename) {
  BIMAGE_DEPTH depth = bimageTypeDepth(im->type);

  if (!tiffInitialized) {
    tiffInit();
  }

  TIFF *tif = TIFFOpen(filename, "w");
  if (!tif) {
    return BIMAGE_ERR;
  }

  TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
  if (bimageTypeChannels(im->type) == 1) {
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
  } else {
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
  }
  TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
  TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, im->width);
  TIFFSetField(tif, TIFFTAG_IMAGELENGTH, im->height);
  TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE,
               bimageDepthSize(bimageTypeDepth(im->type)));
  TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, bimageTypeChannels(im->type));

  if (depth == BIMAGE_F32 || depth == BIMAGE_F64) {
    TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
  } else if (depth == BIMAGE_C32) {
    TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_COMPLEXIEEEFP);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 32);
  } else {
    TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
  }

  uint64_t offs = (bimageDepthSize(bimageTypeDepth(im->type)) / 8) *
                  (uint64_t)bimageTypeChannels(im->type) * im->width;

  uint32_t j;
  for (j = 0; j < im->height; j++) {
    if (TIFFWriteScanline(tif, im->data + offs * j, j, 0) < 0) {
      TIFFClose(tif);
      remove(filename);
      return BIMAGE_ERR;
    }
  }

  TIFFClose(tif);
  return BIMAGE_OK;
}

bimage *bimageOpenTIFF(const char *filename) {
  if (!tiffInitialized) {
    tiffInit();
  }

  bimage *im = NULL;
  TIFF *tif = TIFFOpen(filename, "r");
  if (!tif) {
    return NULL;
  }

  uint32_t w, h;
  uint16_t depth, channels, fmt;

  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
  TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &depth);
  TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &channels);
  TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &fmt);

  if (fmt == SAMPLEFORMAT_IEEEFP) {
    depth =
        depth == 32 ? BIMAGE_F32 : depth == 64 ? BIMAGE_F64 : BIMAGE_UNKNOWN;
  } else if (fmt == SAMPLEFORMAT_COMPLEXIEEEFP) {
    depth = depth == 32 ? BIMAGE_C32 : BIMAGE_UNKNOWN;
  } else if (fmt == SAMPLEFORMAT_UINT) {
    depth = depth == 8
                ? BIMAGE_U8
                : depth == 16 ? BIMAGE_U16
                              : depth == 32 ? BIMAGE_U32 : BIMAGE_UNKNOWN;
  } else {
    goto error0;
  }

  im = bimageCreate(w, h, depth | channels);
  if (!im) {
    goto error0;
  }

  tstrip_t strip;
  tdata_t *buf = bAlloc(TIFFStripSize(tif));
  if (!buf) {
    goto error0;
  }

  for (strip = 0; strip < TIFFNumberOfStrips(tif); strip++) {
    tsize_t n = TIFFReadEncodedStrip(tif, strip, buf, -1);
    if (n <= 0) {
      goto error1;
    }

    memcpy(im->data + TIFFStripSize(tif) * strip, buf, n);
  }

  bFree(buf);
  TIFFClose(tif);
  return im;

error1:
  bFree(buf);
error0:
  if (im)
    bimageRelease(im);
  TIFFClose(tif);
  return NULL;
}
