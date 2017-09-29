bimage
======

[![Build Status](https://travis-ci.org/zshipko/bimage.svg?branch=master)](https://travis-ci.org/zshipko/bimage)

`bimage` is a basic image library for C.

* Simple API
* Handles 8, 16 and 32 bit images
* A choice between in-memory and memory-mapped image data

## Dependencies

* GNU make
* libtiff
* C99 compiler
* check (for building tests)

## Installation

    make
    make install

To uninstall:

    make uninstall

## Formats

* [R/W] 8, 16 and 32 bit TIFF files
* [R/W] 8 bit PNG files
* [R] 8 and 16 bit PNG, JPG, PSD, GIF, BMP, TGA files (using `stb_image`)
* [R/W] 32 bit (floating point) HDR files (using `stb_image`)

## Usage
Loading an image:

    bimage *im = bimageOpen("my-image.tiff");
    if (!im){
        // Handle invalid image
    }

Resizing an image:

    bimage *resized = bimageResize(NULL, im, 500, 0);
    if (!resized){
        // Handle resize error
    }

Get a pixel:

    bimagePixel px;
    if (bimageGet(resized, x, y, &px) != BIMAGE_OK){
        // Handle out-of-bounds
    }

Set a pixel:

    if (bimageSet(resized, x, y, px) != BIMAGE_OK){
        // Handle out of bounds
    }

Iterate over a range of pixels:

    bimageIter(im, 0, 0, im->width, im->height, 1, 1){
        // Do something
    }

Release the images:

    bimageRelease(resized);
    bimageRelease(im);

See `src/bimage.h` for more information.
