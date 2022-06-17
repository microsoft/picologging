#include <Python.h>
#include <structmember.h>
#include <cstddef>
#include "compat.hxx"
#include "logrecord.hxx"
#include <unordered_map>

#ifndef PICOLOGGING_LOGGER_H
#define PICOLOGGING_LOGGER_H

typedef struct LoggerT {
    PyObject_HEAD
    PyObject *name;
    unsigned short level;
    PyObject *parent;
    bool propagate;
    PyObject *handlers;
    bool disabled;
    PyObject *filters;
    bool enabledForCritical = false;
    bool enabledForError = false;
    bool enabledForWarning = false;
    bool enabledForInfo = false;
    bool enabledForDebug = false;
} Logger ;

int Logger_init(Logger *self, PyObject *args, PyObject *kwds);
PyObject* Logger_setLevel(Logger *self, PyObject *args);
PyObject* Logger_getEffectiveLevel(Logger *self);
PyObject* Logger_dealloc(Logger *self);

PyObject* Logger_debug(Logger *self, PyObject *const *args, Py_ssize_t nargs, PyObject *kwds);
PyObject* Logger_info(Logger *self, PyObject *const *args, Py_ssize_t nargs, PyObject *kwds);
PyObject* Logger_warning(Logger *self, PyObject *const *args, Py_ssize_t nargs, PyObject *kwds);
PyObject* Logger_fatal(Logger *self, PyObject *const *args, Py_ssize_t nargs, PyObject *kwds);
PyObject* Logger_error(Logger *self, PyObject *const *args, Py_ssize_t nargs, PyObject *kwds);
PyObject* Logger_critical(Logger *self, PyObject *const *args, Py_ssize_t nargs, PyObject *kwds);
PyObject* Logger_exception(Logger *self, PyObject *args, PyObject *kwds);
PyObject* Logger_log(Logger *self, PyObject *args, PyObject *kwds);

LogRecord* Logger_logMessageAsRecord(Logger* self, unsigned short level, PyObject *msg, PyObject *args, PyObject * exc_info, PyObject *extra, PyObject *stack_info, int stacklevel=1);

extern PyTypeObject LoggerType;
#define Logger_CheckExact(op) Py_IS_TYPE(op, &LoggerType)

#endif // PICOLOGGING_LOGGER_H