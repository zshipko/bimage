#ifndef BIMAGE_NUMPY_HEADER
#define BIMAGE_NUMPY_HEADER

#include <Python.h>
#include <bimage.h>

PyObject *bimageToNumpy(bimage *im);
bimage *bimageFromNumpy(PyObject *obj);

#endif
