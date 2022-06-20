/*
 * from CPython 3.10 object.h
 * See https://www.python.org/doc/copyright/
 */

#include <Python.h>

#ifndef COMPAT_H
#define COMPAT_H

#ifndef _PyObject_CAST
#define _PyObject_CAST(op) ((PyObject*)(op))
#endif

#ifndef _PyObject_CAST_CONST
#define _PyObject_CAST_CONST(op) ((const PyObject*)(op))
#endif

#ifndef _PyVarObject_CAST
#define _PyVarObject_CAST(op) ((PyVarObject*)(op))
#endif

#ifndef _PyVarObject_CAST_CONST
#define _PyVarObject_CAST_CONST(op) ((const PyVarObject*)(op))
#endif

#ifndef Py_IS_TYPE
static inline int _Py_IS_TYPE(const PyObject *ob, const PyTypeObject *type) {
    return ob->ob_type == type;
}
#define Py_IS_TYPE(ob, type) _Py_IS_TYPE(_PyObject_CAST_CONST(ob), type)
#endif

#ifndef Py_SET_SIZE
static inline void _Py_SET_SIZE(PyVarObject *ob, Py_ssize_t size) {
    ob->ob_size = size;
}
#define Py_SET_SIZE(ob, size) _Py_SET_SIZE(_PyVarObject_CAST(ob), size)
#endif

#define PYUNICODE_ENDSWITH(ob, suffix) (PyUnicode_Tailmatch(ob, suffix, PyUnicode_GET_LENGTH(ob) - 1, PyUnicode_GET_LENGTH(ob), +1) > 0)

#if PY_VERSION_HEX >= 0x03090000
#define PyObject_CallMethod_ONEARG(ob, name, arg) PyObject_CallMethodOneArg(ob, name, arg)
#else
#define PyObject_CallMethod_ONEARG(ob, name, arg) PyObject_CallMethodObjArgs(ob, name, arg, NULL)
#endif

#if PY_VERSION_HEX >= 0x03090000
#define PyObject_CallMethod_NOARGS(ob, name) PyObject_CallMethodNoArgs(ob, name)
#else
#define PyObject_CallMethod_NOARGS(ob, name) PyObject_CallMethodObjArgs(ob, name, NULL)
#endif

#if PY_VERSION_HEX >= 0x030b0000 // Python 3.11.0
#define PyFrame_GETBACK(f) PyFrame_GetBack(f)
#define PyFrame_GETCODE(f) PyFrame_GetCode(f)
#define PyFrame_GETLINENO(f) PyFrame_GetLineNumber(f)
#else
#define PyFrame_GETBACK(f) f->f_back
#define PyFrame_GETCODE(f) f->f_code
#define PyFrame_GETLINENO(f) f->f_lineno
#endif

#endif // COMPAT_H