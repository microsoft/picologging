#include <Python.h>
#include "handler.hxx"

#ifndef PICOLOGGING_STREAMHANDLER_H
#define PICOLOGGING_STREAMHANDLER_H

typedef struct {
    Handler handler;
    PyObject* stream;
    PyObject* terminator;
    PyObject* _const_write;
    PyObject* _const_flush;
    bool stream_has_flush;
} StreamHandler;
PyObject* StreamHandler_emit(StreamHandler* self, PyObject* const* args, Py_ssize_t nargs);

extern PyTypeObject StreamHandlerType;
#define StreamHandler_CheckExact(op) Py_IS_TYPE(op, &StreamHandlerType)
#endif // PICOLOGGING_STREAMHANDLER_H