#include <Python.h>
#include <structmember.h>
#include <cstddef>
#include <unordered_map>
#include <string>

#include "compat.hxx"

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
    Field_Asctime,
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
    int style;
    PyObject* _const_format;
    FormatFragment fragments[1];
} FormatStyle;

int FormatStyle_init(FormatStyle *self, PyObject *args, PyObject *kwds);
PyObject* FormatStyle_usesTime(FormatStyle *self);
PyObject* FormatStyle_validate(FormatStyle *self);
PyObject* FormatStyle_format(FormatStyle *self, PyObject *record);
PyObject* FormatStyle_dealloc(FormatStyle *self);
PyObject* FormatStyle_new(PyTypeObject *type, PyObject *args, PyObject *kwds);

extern PyTypeObject FormatStyleType;
#define FormatStyle_CheckExact(op) Py_IS_TYPE(op, &FormatStyleType)

typedef std::unordered_map<std::string, FragmentType> FieldMap;
#endif // PICOLOGGING_FORMATSTYLE_H