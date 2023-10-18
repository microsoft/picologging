#include <Python.h>
#include "handler.hxx"

#ifndef PICOLOGGING_QUEUEHANDLER_H
#define PICOLOGGING_QUEUEHANDLER_H

typedef struct {
    Handler handler;
    PyObject* queue;
    PyObject* _const_put_nowait;
    PyObject* _const_prepare;
    PyObject* _const_enqueue;
} QueueHandler;

PyObject* QueueHandler_enqueue(QueueHandler* self, PyObject* record);
PyObject* QueueHandler_prepare(QueueHandler* self, PyObject* record);
PyObject* QueueHandler_emit(QueueHandler* self, PyObject* record);

extern PyTypeObject QueueHandlerType;
#define QueueHandler_CheckExact(op) Py_IS_TYPE(op, &QueueHandlerType)
#endif // PICOLOGGING_QUEUEHANDLER_H