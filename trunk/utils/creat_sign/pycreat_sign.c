#include "pycreat_sign.h"
#include <assert.h>

PyObject* wrap_creat_sign(PyObject* self, PyObject* args) 
{
    assert (self != NULL || self == NULL);
    uint32_t sign1 = 0;
    uint32_t sign2 = 0;
    const char* str = NULL;
    uint32_t length = 0;
    if (! PyArg_ParseTuple(args, "sI", &str, &length))
    {
        return NULL;
    }
    creat_sign_64(str, length, &sign1, &sign2);
    PyObject* pTuple = PyTuple_New(2);
    PyTuple_SetItem(pTuple, 0, Py_BuildValue("I", sign1));
    PyTuple_SetItem(pTuple, 1, Py_BuildValue("I", sign2));
    return pTuple;
}

static PyMethodDef creat_signMethods[] = 
{
    {"creat_sign", wrap_creat_sign, METH_VARARGS, "create sign tuple(int int) for row data"},
    {NULL, NULL, 0, NULL}
};

void initcreat_sign() 
{
    PyObject* m;
    m = Py_InitModule("creat_sign", creat_signMethods);
}

