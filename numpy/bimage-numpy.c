#include "bimage-numpy.h"

#include <numpy/arrayobject.h>

int numpy_type(bimage *im){
    switch bimageTypeDepth(im->type) {
    case BIMAGE_U8:
        return NPY_UINT8;
    case BIMAGE_U16:
        return NPY_UINT16;
    case BIMAGE_U32:
        return NPY_UINT32;
    case BIMAGE_F32:
        return NPY_FLOAT;
    }

    return -1;
}

PyObject* bimageToNumpy(bimage *im){
    int t = numpy_type(im);
    if (t < 0){
        return NULL;
    }

    int ndims = bimageTypeChannels(im->type) == 1 ? 2 : 3;
    npy_intp dims[ndims];
    dims[0] = im->height;
    dims[1] = im->width;
    if (ndims == 3){
        dims[3] = bimageTypeChannels(im->type);
    }

    return PyArray_SimpleNewFromData(ndims, dims, numpy_type(im), im->data);
}
