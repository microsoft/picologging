#include <Python.h>
#include <structmember.h>
#include <cstddef>
#include "compat.hxx"
#include "logrecord.hxx"
#include "filterer.hxx"
#include <unordered_map>
#include "streamhandler.hxx"

#ifndef PICOLOGGING_LOGGER_H
#define PICOLOGGING_LOGGER_H

typedef struct LoggerT {
    Filterer filterer;
    PyObject *name;
    unsigned short level;
    PyObject *parent;
    PyObject *propagate;
    PyObject *handlers;
    PyObject *manager;
    bool disabled;
    bool enabledForCritical = false;
    bool enabledForError = false;
    bool enabledForWarning = false;
    bool enabledForInfo = false;
    bool enabledForDebug = false;

    // Constant strings.
    PyObject* _const_handle;
    PyObject* _const_level;
    PyObject* _const_unknown;
    PyObject* _const_exc_info;
    PyObject* _const_extra;
    PyObject* _const_stack_info;
    PyObject* _const_line_break;

    StreamHandler* _fallback_handler;
} Logger ;

int Logger_init(Logger *self, PyObject *args, PyObject *kwds);
PyObject* Logger_setLevel(Logger *self, PyObject *args);
PyObject* Logger_getEffectiveLevel(Logger *self);
PyObject* Logger_dealloc(Logger *self);
PyObject* Logger_addHandler(Logger *self, PyObject *handler);

PyObject* Logger_logAndHandle(Logger *self, PyObject *args, PyObject *kwds, unsigned short level);
PyObject* Logger_debug(Logger *self, PyObject *args, PyObject *kwds);
PyObject* Logger_info(Logger *self, PyObject *args, PyObject *kwds);
PyObject* Logger_warning(Logger *self, PyObject *args, PyObject *kwds);
PyObject* Logger_fatal(Logger *self, PyObject *args, PyObject *kwds);
PyObject* Logger_error(Logger *self, PyObject *args, PyObject *kwds);
PyObject* Logger_critical(Logger *self, PyObject *args, PyObject *kwds);
PyObject* Logger_exception(Logger *self, PyObject *args, PyObject *kwds);
PyObject* Logger_log(Logger *self, PyObject *args, PyObject *kwds);

LogRecord* Logger_logMessageAsRecord(Logger* self, unsigned short level, PyObject *msg, PyObject *args, PyObject * exc_info, PyObject *extra, PyObject *stack_info, int stacklevel=1);

extern PyTypeObject LoggerType;
#define Logger_CheckExact(op) Py_IS_TYPE(op, &LoggerType)
#define Logger_Check(op) PyObject_TypeCheck(op, &LoggerType)

#endif // PICOLOGGING_LOGGER_H