bimage
======

[![Build Status](https://travis-ci.org/zshipko/bimage.svg?branch=master)](https://travis-ci.org/zshipko/bimage)

`bimage` is a basic image library for C.

* Simple API
* Handles a wide range of datatypes
    - Integers, uint8, uint16, uint32
    - Floating point: float, double
    - Complex: complex32
* Multithreaded iterators
* In-memory and memory mapped image data

## Dependencies

* GNU make
* C99 compiler
* libcheck (ony when building tests)

## Optional Dependencies

* libtiff (enabled by default)
    - set the environment variable `TIFF=NO` before building to disable libtiff
* NumPy (for bimage-numpy)
* GTK 3.0 (for bimage-gtk)
* OpenCV (for bimage-opencv)

## Installation

    make
    make install

To uninstall:

    make uninstall

## Formats

* [R/W] 8, 16 and 32 bit TIFFs
* [R/W] 8 bit PNGs
* [R/W] 8 bit JPGs
* [R] 8 and 16 bit PNG, PSD, GIF, BMP, TGA files (using `stb_image`)
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
    if (bimageGetPixel(resized, x, y, &px) != BIMAGE_OK){
        // Handle out-of-bounds
    }

Set a pixel:

    if (bimageSetPixel(resized, x, y, px) != BIMAGE_OK){
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
