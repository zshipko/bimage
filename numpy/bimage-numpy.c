#include "bimage-numpy.h"

#include <numpy/arrayobject.h>

static int _numpyType(bimage *im){
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

static int _bimageType(int ntype){
    switch (ntype) {
    case NPY_UINT8:
        return BIMAGE_U8;
    case NPY_UINT16:
        return BIMAGE_U16;
    case NPY_UINT32:
        return BIMAGE_U32;
    case NPY_FLOAT:
        return BIMAGE_F32;
    }

    return BIMAGE_UNKNOWN;
}

PyObject* bimageToNumpy(bimage *im){
    int t = _numpyType(im);
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

    return PyArray_SimpleNewFromData(ndims, dims, t, im->data);
}

bimage* bimageFromNumpy(PyObject *obj){
    if (!PyArray_Check(obj)){
        return NULL;
    }

    // Get image dimensions
    int ndim = PyArray_NDIM(obj);
    if (ndim < 2){
        return NULL;
    }

    npy_intp *dims = PyArray_DIMS(obj);
    uint32_t width = dims[1];
    uint32_t height = dims[0];
    int channels = ndim == 2 ? 1 : dims[2];

    // Get image type
    PyArray_Descr *dtype = PyArray_DTYPE((PyArrayObject*)obj);
    int type = _bimageType(dtype->type_num);
    if (type == BIMAGE_UNKNOWN) {
        return NULL;
    }

    return bimageCreateWithData(width, height, type | channels, PyArray_DATA(obj), false, false);
}
