#include "bimage-opencv.h"

static cv::Scalar
bimagePixelToScalar(bimagePixel *px)
{
    return cv::Scalar(px->data.f[0], px->data.f[1], px->data.f[2], px->data.f[3]);
}

static bimagePixel
bimagePixelFromScalar(cv::Scalar src)
{
    return bimagePixelCreate(src[0], src[1], src[2], src[3], BIMAGE_UNKNOWN);
}

static int
bimageTypeToMatType(BIMAGE_TYPE t)
{
    uint16_t channels = bimageTypeChannels(t);
    uint16_t depth = bimageTypeDepth(t);

    switch (depth) {
    case BIMAGE_U8:
        return CV_8UC(channels);
    case BIMAGE_U16:
        return CV_16UC(channels);
    case BIMAGE_U32:
        return CV_32SC(channels);
    case BIMAGE_F32:
        return CV_32FC(channels);
    case BIMAGE_F64:
    case BIMAGE_C32:
        return CV_64FC(channels);
    default:
        return -1;
    }
}

static int
bimageTypeFromMatType(int channels, int depth)
{
    switch (depth) {
    case CV_8U:
        return BIMAGE_U8 | channels;
    case CV_16U:
        return BIMAGE_U16 | channels;
    case CV_32S:
        return BIMAGE_U32 | channels;
    case CV_32F:
        return BIMAGE_F32 | channels;
    case CV_64F:
        return BIMAGE_F64 | channels;
    }

    return -1;
}

cv::Mat
bimageToMat(bimage *im)
{
    int t;
    if (!im || (t=bimageTypeToMatType(im->type)) < 0){
        return cv::Mat();
    }

    return cv::Mat(im->height, im->width, t, im->data);
}

bimage *
bimageFromMat(cv::Mat& m)
{
    int channels = m.channels();
    int depth = m.depth();

    int t = bimageTypeFromMatType(channels, depth);
    if (t < 0){
        return NULL;
    }

    return bimageCreateWithData(m.cols, m.rows, t, m.data, false, false);
}

BIMAGE_CV_API
bimagePixel
bimageVariance(bimage *_image)
{
    cv::Mat image = bimageToMat(_image);
    cv::Scalar mean, stddev;

    try {
        cv::meanStdDev(image, mean, stddev);
    } catch (cv::Exception exc) {
        return BIMAGE_PIXEL_INIT;
    }

    bimagePixel px = bimagePixelFromScalar(stddev);
    bimagePixelMul(&px, px);
    return px;
}

BIMAGE_CV_API
bimagePixel
bimageMean(bimage *_image)
{
    cv::Mat image = bimageToMat(_image);
    cv::Scalar mean, stddev;

    try {
        cv::mean(image, mean);
    } catch (cv::Exception exc) {
        return BIMAGE_PIXEL_INIT;
    }

    return bimagePixelFromScalar(mean);
}

BIMAGE_CV_API
bimage *
bimageMatchTemplate(bimage *_dest, bimage *_image, bimage *_templ, int method, bimage *_mask)
{

    bimage *_tmp;
    int depth = bimageTypeDepth(_image->type);
    if (depth != BIMAGE_F32 && depth != BIMAGE_U8){
        _tmp = bimageConvertDepth(NULL, _image, BIMAGE_F32);
    } else {
        _tmp = _image;
    }

    cv::Mat image = bimageToMat(_tmp);
    cv::Mat templ = bimageToMat(_templ);
    cv::Mat mask = bimageToMat(_mask);

    int w = image.cols - templ.cols + 1;
    int h = image.rows - templ.rows + 1;
    bimage *_tmpdest = BIMAGE_CREATE_DEST(_dest, w, h, BIMAGE_F32 | 1);

    cv::Mat dest = bimageToMat(_dest);
    try {
        cv::matchTemplate(image, templ, dest, method, mask);
    } catch (cv::Exception exc){
        goto error;
    }

    if (_tmp != _image){
        bimageRelease(_tmp);
    }

    return _tmpdest;

error:
    if (_tmp != _image){
        bimageRelease(_tmp);
    }
    if (_tmpdest != _dest){
        bimageRelease(_tmpdest);
    }
    return NULL;
}


