#include <Python.h>
#include <structmember.h>
#include <cstddef>
#include "compat.hxx"

#ifndef PICOLOGGING_LOGGER_H
#define PICOLOGGING_LOGGER_H

typedef struct {
    PyObject_HEAD
    PyObject *name;
    unsigned short level;
    PyObject *parent;
    bool propagate;
    PyObject *handlers;
    bool disabled;
    PyObject *_cache;
    PyObject *filters;
} Logger;

int Logger_init(Logger *self, PyObject *args, PyObject *kwds);
PyObject* Logger_setLevel(Logger *self, PyObject *args);
PyObject* Logger_getEffectiveLevel(Logger *self);
PyObject* Logger_dealloc(Logger *self);

extern PyTypeObject LoggerType;
#define Logger_CheckExact(op) Py_IS_TYPE(op, &LoggerType)

#endif // PICOLOGGING_LOGGER_H