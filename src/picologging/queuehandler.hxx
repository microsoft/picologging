#include <Python.h>
#include "handler.hxx"

#ifndef PICOLOGGING_QUEUEHANDLER_H
#define PICOLOGGING_QUEUEHANDLER_H

typedef struct {
    Handler handler;
    PyObject* queue;
    PyObject* _const_put_nowait;
} QueueHandler;
PyObject* QueueHandler_enqueue(QueueHandler* self, PyObject* const* args, Py_ssize_t nargs);
PyObject* QueueHandler_prepare(QueueHandler* self, PyObject* const* args, Py_ssize_t nargs);
PyObject* QueueHandler_emit(QueueHandler* self, PyObject* const* args, Py_ssize_t nargs);

extern PyTypeObject QueueHandlerType;
#define QueueHandler_CheckExact(op) Py_IS_TYPE(op, &QueueHandlerType)
#endif // PICOLOGGING_QUEUEHANDLER_H