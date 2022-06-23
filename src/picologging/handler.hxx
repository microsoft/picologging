#include <Python.h>
#include "filterer.hxx"
#include <mutex>

#ifndef PICOLOGGING_HANDLER_H
#define PICOLOGGING_HANDLER_H

typedef struct {
    Filterer filterer;
    PyObject *name;
    unsigned short level;
    PyObject *formatter;
    std::recursive_mutex *lock;
    PyObject* _const_emit;
    PyObject* _const_format;
} Handler;

int Handler_init(Handler *self, PyObject *args, PyObject *kwds);
PyObject* Handler_dealloc(Handler *self);
PyObject* Handler_emit(Handler *self, PyObject *record);
PyObject* Handler_handle(Handler *self, PyObject *record);
PyObject* Handler_setLevel(Handler *self, PyObject *level);
PyObject* Handler_setFormatter(Handler *self, PyObject *formatter);
PyObject* Handler_format(Handler *self, PyObject *record);
PyObject* Handler_acquire(Handler *self);
PyObject* Handler_release(Handler *self);

extern PyTypeObject HandlerType;
#define Handler_CheckExact(op) Py_IS_TYPE(op, &HandlerType)
#define Handler_Check(op) PyObject_TypeCheck(op, &HandlerType)

#endif // PICOLOGGING_HANDLER_H
