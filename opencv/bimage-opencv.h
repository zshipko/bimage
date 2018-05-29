#ifndef __BIMAGE_OPENCV
#define __BIMAGE_OPENCV

#include <opencv2/opencv.hpp>
#include "bimage.h"

#ifndef __cplusplus
#define BIMAGE_CV_API extern "C"
#else
#define BIMAGE_CV_API
#endif

#ifdef __cplusplus
cv::Mat bimageToMat(bimage*);
bimage *bimageFromMat(cv::Mat&);
#endif

typedef enum cv::TemplateMatchModes bimageTemplateMatchMode;

BIMAGE_CV_API
bimagePixel
bimageCvVariance(bimage *_image);

BIMAGE_CV_API
bimagePixel
bimageCvMean(bimage *_image);

BIMAGE_CV_API
bimage *
bimageCvMatchTemplate(bimage *_image, bimage *_templ, int method, bimage *_mask);

#endif // __BIMAGE_OPENCV
