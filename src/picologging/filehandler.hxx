#include <Python.h>
#include "handler.hxx"

#ifndef PICOLOGGING_FILEHANDLER_H
#define PICOLOGGING_FILEHANDLER_H

typedef struct {
    Handler handler;
    PyObject* stream;
    PyObject* terminator;
    PyObject* _const_io_mod;
} FileHandler;
PyObject* FileHandler_emit(FileHandler* self, PyObject* const* args, Py_ssize_t nargs);
PyObject* FileHandler_flush(FileHandler* self);
PyObject* FileHandler_close(FileHandler* self);

extern PyTypeObject FileHandlerType;
#define FileHandler_CheckExact(op) Py_IS_TYPE(op, &FileHandlerType)
#endif // PICOLOGGING_FILEHANDLER_H
