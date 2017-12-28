#include "bimage-opencv.hpp"

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
    deffault:
        return -1;
    }
}

cv::Mat
bimageToMat(bimage *im)
{
    int t;
    if ((t=bimageTypeToMatType(im->type)) < 0){
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
