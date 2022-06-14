#include <Python.h>
#include <structmember.h>
#include <cstddef>
#include <unordered_map>
#include <string>

#ifndef PICOLOGGING_FORMATSTYLE_H
#define PICOLOGGING_FORMATSTYLE_H

enum FragmentType {
    Field_Name = 1, // Skip 0 incase there's an offset error
    Field_Msg,
    Field_Args,
    Field_LevelNo,
    Field_LevelName,
    Field_Pathname,
    Field_Filename,
    Field_Module,
    Field_Lineno,
    Field_FuncName,
    Field_Created,
    Field_Msecs,
    Field_RelativeCreated,
    Field_Thread,
    Field_ThreadName,
    Field_ProcessName,
    Field_Process,
    Field_ExcInfo,
    Field_ExcText,
    Field_StackInfo,
    Field_Message,
    // TODO : asctime
    // Special field
    Field_Unknown,
    LiteralFragment,
};

typedef struct {
    FragmentType field;
    PyObject *fragment;
} FormatFragment;

typedef struct {
    PyObject_VAR_HEAD
    PyObject *fmt;
    PyObject *defaults;
    bool usesDefaultFmt;
    FormatFragment fragments[1];
} PercentStyle;

int PercentStyle_init(PercentStyle *self, PyObject *args, PyObject *kwds);
PyObject* PercentStyle_usesTime(PercentStyle *self);
PyObject* PercentStyle_validate(PercentStyle *self);
PyObject* PercentStyle_format(PercentStyle *self, PyObject *record);
PyObject* PercentStyle_dealloc(PercentStyle *self);
PyObject* PercentStyle_new(PyTypeObject *type, PyObject *args, PyObject *kwds);

PyAPI_DATA(PyTypeObject) PercentStyleType;

#ifdef Py_IS_TYPE
#define PercentStyle_CheckExact(op) Py_IS_TYPE(op, &PercentStyleType)
#else
#define PercentStyle_CheckExact(op) (Py_TYPE(op) == &PercentStyleType)
#endif

// Python <3.9 backward compat
#ifndef Py_SET_SIZE
static inline void _Py_SET_SIZE(PyVarObject *ob, Py_ssize_t size) {
    ob->ob_size = size;
}
#define Py_SET_SIZE(ob, size) _Py_SET_SIZE(_PyVarObject_CAST(ob), size)
#endif

typedef std::unordered_map<std::string, FragmentType> FieldMap;
#endif // PICOLOGGING_FORMATSTYLE_H