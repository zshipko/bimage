# bimage-opencv

Conversion between bimage and opencv image containers.

## API

- `Mat bimageToMat(bimage*)`
    * Convert a `bimage` to `cv::Mat`
- `bimage *bimageFromMat(Mat&)`
    * Convert a `cv::Mat` to `bimage`

## Building

To build the shared library:

    make

To install:

    make install

To uninstall

    make uninstall

## Dependencies

- [bimage](https://github.com/zshipko/bimage)
- [opencv](https://github.com/opencv/opencv)
