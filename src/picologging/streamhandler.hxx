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

extern PyTypeObject StreamHandlerType;

#endif // PICOLOGGING_STREAMHANDLER_H