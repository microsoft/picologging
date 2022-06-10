#include "formatter.h"
#include "formatstyle.h"
#include "logrecord.h"

PyObject* Formatter_init(Formatter *self, PyObject *args, PyObject *kwds){
    PyObject *fmt = Py_None, *dateFmt = Py_None;
    char style = '%';
    bool validate = true;
    static char *kwlist[] = {"fmt", "datefmt", "style", "validate", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OOCp", kwlist, &fmt, &dateFmt, &style, &validate))
        return NULL;

    PyObject* styleType = nullptr;

    switch (style){
        case '%':
            /* Call the class object. */
            assert(PyType_Ready(&PercentStyleType) == 0);
            styleType = (PyObject*)&PercentStyleType;
            break;
        default:
            PyErr_SetString(PyExc_ValueError, "Unsupported style");
            return nullptr;
    }

    self->style = PyObject_CallFunctionObjArgs(styleType, fmt, NULL);
    if (self->style == nullptr){ // Got exception in PercentStyle.__init__()
        return nullptr;
    }

    self->fmt = ((PercentStyle*)(self->style))->fmt;
    Py_IncRef(fmt);

    self->dateFmt = dateFmt;
    Py_IncRef(dateFmt);

    if (validate){
        if (PyObject_CallMethod(self->style, "validate", NULL) == nullptr){
            Py_DECREF(self->style);
            return nullptr;
        }
    }

    Py_IncRef(self->style) ; // TODO: This is a new reference, so check if this is required.
    return (PyObject*)self;
}

PyObject* Formatter_format(Formatter *self, PyObject *record){
    if (LogRecord_CheckExact(record)){
        LogRecord* logRecord = (LogRecord*)record;
        logRecord->message = LogRecord_getMessage(logRecord);
        Py_INCREF(logRecord->message);
        if (PercentStyle_CheckExact(self->style)){
            return PercentStyle_format((PercentStyle*)self->style, record);
        } else {
            return PyObject_CallMethodObjArgs(self->style, PyUnicode_FromString("format"), record, NULL);
            // TODO : format exc_info, exc_text and stack_info.
        }
    } else {
        PyErr_SetString(PyExc_TypeError, "Argument must be a LogRecord");
        return nullptr;
    }
    return Py_None; // TODO: Implement.
}

PyObject* Formatter_dealloc(Formatter *self) {
    Py_XDECREF(self->style);
    Py_XDECREF(self->fmt);
    Py_XDECREF(self->dateFmt);
    Py_TYPE(self)->tp_free((PyObject*)self);
    return NULL;
}

static PyMethodDef Formatter_methods[] = {
    {"format", (PyCFunction)Formatter_format, METH_O, "Format record into log event string"},
    {NULL}
};

static PyMemberDef Formatter_members[] = {
    {"_fmt", T_OBJECT_EX, offsetof(Formatter, fmt), 0, "Format string"},
    {"_style", T_OBJECT_EX, offsetof(Formatter, style), 0, "String style formatter"},
    {"datefmt", T_OBJECT_EX, offsetof(Formatter, dateFmt), 0, "Date format string"},
    {NULL}
};

PyTypeObject FormatterType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "picologging.Formatter",
    .tp_doc = PyDoc_STR("Formatter for log records."),
    .tp_basicsize = sizeof(Formatter),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc)Formatter_init,
    .tp_dealloc = (destructor)Formatter_dealloc,
    .tp_repr = (reprfunc)PyObject_Repr,
    .tp_members = Formatter_members,
    .tp_methods = Formatter_methods,
    .tp_getattro = PyObject_GenericGetAttr,
    .tp_setattro = PyObject_GenericSetAttr,
};