#ifndef __BIMAGE_OPENCV
#define __BIMAGE_OPENCV

#include <opencv2/opencv.hpp>
#include <bimage.h>

#ifdef __cplusplus
cv::Mat bimageToMat(bimage*);
bimage *bimageFromMat(cv::Mat&);
#endif

extern "C" {

double
bimageVariance(bimage *_image);

bimage *
bimageMatchTemplate(bimage *_image, bimage *_templ, int method, bimage *_mask);

}

#endif // __BIMAGE_OPENCV
