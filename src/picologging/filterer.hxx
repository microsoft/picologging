#include <Python.h>
#include <structmember.h>

#ifndef PICOLOGGING_FILTERER_H
#define PICOLOGGING_FILTERER_H

typedef struct {
    PyObject_HEAD
    PyObject *filters;
} Filterer;

int Filterer_init(Filterer *self, PyObject *args, PyObject *kwds);
PyObject* Filterer_filter(Filterer* self, PyObject *record);
PyObject* Filterer_dealloc(Filterer *self);

extern PyTypeObject FiltererType;
#define Filterer_CheckExact(op) Py_IS_TYPE(op, &FiltererType)

#endif
