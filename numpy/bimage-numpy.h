#ifndef BIMAGE_NUMPY_HEADER
#define BIMAGE_NUMPY_HEADER

#include <bimage.h>
#include <Python.h>

PyObject* bimageToNumpy(bimage *im);
bimage* bimageFromNumpy(PyObject *obj);

#endif
