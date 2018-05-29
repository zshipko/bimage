#include "bimage-cv.h"

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
    deffault:
        return -1;
    }
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

extern "C" double
bimageVariance(bimage *_image)
{
    cv::Mat image = bimageToMat(_image);
    cv::Scalar mean, stddev;

    try {
        cv::meanStdDev(image, mean, stddev);
    } catch (cv::Exception exc) {
        return 0;
    }

    return stddev.val[0] * stddev.val[0];
}

extern "C" bimage *
bimageMatchTemplate(bimage *_image, bimage *_templ, int method, bimage *_mask)
{
    cv::Mat image = bimageToMat(_image);
    cv::Mat templ = bimageToMat(_templ);
    cv::Mat mask = bimageToMat(_mask);

    int w = image.cols - templ.cols + 1;
    int h = image.rows - templ.rows + 1;
    bimage *_dest = bimageCreate(w, h, BIMAGE_F32 | 1);

    cv::Mat dest = bimageToMat(_dest);
    try {
        cv::matchTemplate(image, templ, dest, method, mask);
    } catch (cv::Exception exc){
        return NULL;
    }

    return _dest;
}


