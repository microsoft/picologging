#include <Python.h>
#include <mutex>
#include "filterer.hxx"

#ifndef PICOLOGGING_HANDLER_H
#define PICOLOGGING_HANDLER_H

typedef struct {
    Filterer filterer;
    PyObject *name;
    unsigned short level;
    PyObject *formatter;
    std::mutex lock;
} Handler;

int Handler_init(Handler *self, PyObject *args, PyObject *kwds);
PyObject* Handler_dealloc(Handler *self);
PyObject* Handler_emit(Handler *self, PyObject *record);
PyObject* Handler_handle(Handler *self, PyObject *record);
PyObject* Handler_setLevel(Handler *self, PyObject *level);
PyObject* Handler_setFormatter(Handler *self, PyObject *formatter);

extern PyTypeObject HandlerType;
#define Handler_CheckExact(op) Py_IS_TYPE(op, &HandlerType)
#define Handler_Check(op) PyObject_TypeCheck(op, &HandlerType)

#endif // PICOLOGGING_HANDLER_H
