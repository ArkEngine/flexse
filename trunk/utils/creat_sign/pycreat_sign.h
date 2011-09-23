#ifndef _PYCREAT_SIGN_H_
#define _PYCREAT_SIGN_H_
#include <Python.h>
#include "creat_sign.h"

extern "C" {
    PyObject* wrap_creat_sign(PyObject* self, PyObject* args);
    void initcreat_sign();
};
#endif
