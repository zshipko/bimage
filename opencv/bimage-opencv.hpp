#ifndef __BIMAGE_OPENCV
#define __BIMAGE_OPENCV

#include <opencv2/opencv.hpp>
#include <bimage.h>

cv::Mat bimageToMat(bimage*);
bimage *bimageFromMat(cv::Mat&);

#endif // __BIMAGE_OPENCV
