#include <Python.h>
#include <structmember.h>
#include <cstddef>
#include "compat.hxx"

#ifndef PICOLOGGING_FORMATTER_H
#define PICOLOGGING_FORMATTER_H

typedef struct {
    PyObject_HEAD
    PyObject *fmt;
    PyObject *dateFmt;
    PyObject *style;
    bool usesTime;
    size_t dateFmtMicrosendsPos;        // If %f is specified in dateFmt then points to '%' character, otherwise std::string_view::npos
    size_t dateFmtStrSize;              // Size of the dateFmt without null terminator
    const char* dateFmtStr;             // C-string, null terminated
    PyObject *_const_line_break;
    PyObject *_const_close;
    PyObject *_const_getvalue;
    PyObject *_const_usesTime;
    PyObject *_const_format;
} Formatter;

int Formatter_init(Formatter *self, PyObject *args, PyObject *kwds);
PyObject* Formatter_format(Formatter *self, PyObject *record);
PyObject* Formatter_dealloc(Formatter *self);
PyObject* Formatter_usesTime(Formatter *self);
PyObject* Formatter_formatMessage(Formatter *self, PyObject *record);
PyObject* Formatter_formatStack(Formatter *self, PyObject *stackInfo);

extern PyTypeObject FormatterType;
#define Formatter_CheckExact(op) Py_IS_TYPE(op, &FormatterType)


#endif // PICOLOGGING_FORMATTER_H
