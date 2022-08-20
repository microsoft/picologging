#include <ctime>
#include "picologging.hxx"
#include "formatter.hxx"
#include "formatstyle.hxx"
#include "logrecord.hxx"

PyObject* Formatter_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    Formatter* self = (Formatter*)type->tp_alloc(type, 0);
    if (self != NULL)
    {
        self->fmt = Py_None;
        self->dateFmt = Py_None;
        self->style = Py_None;
        self->_const_line_break = PyUnicode_FromString("\n");
    }
    return (PyObject*)self;
}

int Formatter_init(Formatter *self, PyObject *args, PyObject *kwds){
    PyObject *fmt = nullptr, *dateFmt = nullptr;
    int style = '%';
    int validate = 1;
    static const char *kwlist[] = {"fmt", "datefmt", "style", "validate", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OOCp", const_cast<char**>(kwlist), &fmt, &dateFmt, &style, &validate))
        return -1;

    PyObject* styleType = nullptr;
    if (style == 0) style = '%';

    switch (style){
        case '%':
        case '{':
            /* Call the class object. */
            styleType = (PyObject*)&FormatStyleType;
            break;
        case '$':
            PyErr_Format(PyExc_NotImplementedError, "String Templates are not supported in picologging.");
            return -1;
        default:
            PyErr_Format(PyExc_ValueError, "Unknown style '%c'", style);
            return -1;
    }
    if (fmt == nullptr)
        fmt = Py_None;
    if (dateFmt == nullptr)
        dateFmt = Py_None;
    PyObject * styleCls = PyObject_CallFunctionObjArgs(styleType, fmt, Py_None, PyUnicode_FromFormat("%c", style), NULL);
    if (styleCls == nullptr){
        //PyErr_Format(PyExc_ValueError, "Could not initialize Style formatter class.");
        return -1;
    }

    self->style = styleCls;
    Py_INCREF(self->style);

    self->fmt = ((FormatStyle*)(self->style))->fmt;
    Py_INCREF(self->fmt);

    self->usesTime = (FormatStyle_usesTime((FormatStyle*)self->style) == Py_True);

    self->dateFmt = dateFmt;
    Py_INCREF(self->dateFmt);

    if (dateFmt != Py_None) {
        self->dateFmtStr = PyUnicode_AsUTF8(self->dateFmt);
        if (self->dateFmtStr == nullptr) {
            return -1;
        }
    } else {
        self->dateFmtStr = nullptr;
    }

    if (validate){
        if (PyObject_CallMethod(self->style, "validate", NULL) == nullptr){
            Py_DECREF(self->style);
            Py_DECREF(self->fmt);
            Py_DECREF(self->dateFmt);
            return -1;
        }
    }
    return 0;
}

PyObject* Formatter_format(Formatter *self, PyObject *record){
    if (LogRecord_CheckExact(record)){
        LogRecord* logRecord = (LogRecord*)record;
        LogRecord_writeMessage(logRecord);
        PyObject* result = nullptr;
        if (self->usesTime){
            PyObject * asctime = Py_None;
            std::time_t created = (std::time_t)logRecord->created;
            std::tm *ct = localtime(&created);
            if (self->dateFmt != Py_None){
                char buf[100];
                size_t len = strftime(buf, 100, self->dateFmtStr, ct);
                asctime = PyUnicode_FromStringAndSize(buf, len);
            } else {
                char buf[100];
                size_t len = strftime(buf, 100, "%Y-%m-%d %H:%M:%S", ct);
                asctime = PyUnicode_FromFormat("%s,%03d", buf, logRecord->msecs);
            }

            Py_XDECREF(logRecord->asctime);
            logRecord->asctime = asctime;
            if (asctime == Py_None)
                Py_INCREF(Py_None);
        }

        if (FormatStyle_CheckExact(self->style)){
            result = FormatStyle_format((FormatStyle*)self->style, record);
        } else {
            result = PyObject_CallMethod_ONEARG(self->style, PyUnicode_FromString("format"), record);
        }
        if (result == nullptr)
            return nullptr;

        if (logRecord->excInfo != Py_None && logRecord->excText == Py_None){
            if (!PyTuple_Check(logRecord->excInfo)) {
                PyErr_Format(PyExc_TypeError, "LogRecord.excInfo must be a tuple.");
                return nullptr;
            }
            PyObject* mod = PICOLOGGING_MODULE(); // borrowed reference
            PyObject* modDict = PyModule_GetDict(mod); // borrowed reference
            PyObject* print_exception = PyDict_GetItemString(modDict, "print_exception"); // PyDict_GetItemString returns a borrowed reference
            Py_XINCREF(print_exception);
            PyObject* sio_cls = PyDict_GetItemString(modDict, "StringIO");
            Py_XINCREF(sio_cls);
            PyObject* sio = PyObject_CallFunctionObjArgs(sio_cls, NULL);
            if (sio == nullptr){
                Py_XDECREF(sio_cls);
                Py_XDECREF(print_exception);
                return nullptr; // Got exception in StringIO.__init__()
            }
            if (PyObject_CallFunctionObjArgs(
                print_exception,
                PyTuple_GetItem(logRecord->excInfo, 0), 
                PyTuple_GetItem(logRecord->excInfo, 1), 
                PyTuple_GetItem(logRecord->excInfo, 2), 
                Py_None,
                sio,
                NULL) == nullptr)
            {
                Py_XDECREF(sio_cls);
                Py_XDECREF(print_exception);
                return nullptr; // Got exception in print_exception()
            }
            PyObject* s = PyObject_CallMethod_NOARGS(sio, PyUnicode_FromString("getvalue"));
            if (s == nullptr){
                Py_XDECREF(sio);
                Py_XDECREF(sio_cls);
                Py_XDECREF(print_exception);
                return nullptr; // Got exception in StringIO.getvalue()
            }
            
            PyObject_CallMethod_NOARGS(sio, PyUnicode_FromString("close"));
            Py_DECREF(sio);
            Py_DECREF(sio_cls);
            Py_DECREF(print_exception);
            if (PYUNICODE_ENDSWITH(s, self->_const_line_break)){
                PyObject* s2 = PyUnicode_Substring(s, 0, PyUnicode_GetLength(s) - 1);
                Py_DECREF(s);
                s = s2;
            }
            Py_XDECREF(logRecord->excText);
            logRecord->excText = s; // Use borrowed ref
        }
        if (logRecord->excText != Py_None){
            if (!PYUNICODE_ENDSWITH(result, self->_const_line_break)){
                PyUnicode_Append(&result, self->_const_line_break);
            }
            PyUnicode_Append(&result, logRecord->excText);
        }
        if (logRecord->stackInfo != Py_None && logRecord->stackInfo != Py_False){
            if (!PYUNICODE_ENDSWITH(result, self->_const_line_break)){
                PyUnicode_Append(&result, self->_const_line_break);
            }
            if (PyUnicode_Check(logRecord->stackInfo)){
                PyUnicode_Append(&result, logRecord->stackInfo);
            } else {
                PyObject* s = PyObject_Str(logRecord->stackInfo);
                if (s == nullptr){
                    return nullptr; // Got exception in str(stackInfo)
                }
                PyUnicode_Append(&result, s);
                Py_DECREF(s);
            }
        }
        return result;
    } else {
        PyErr_SetString(PyExc_TypeError, "Argument must be a LogRecord");
        return nullptr;
    }
}

PyObject* Formatter_usesTime(Formatter *self) {
    if (FormatStyle_CheckExact(self->style)){
        return FormatStyle_usesTime((FormatStyle*)self->style);
    } else {
        return PyObject_CallMethod_NOARGS(self->style, PyUnicode_FromString("usesTime"));
    }
}

PyObject* Formatter_formatMessage(Formatter *self, PyObject* record){
    return PyObject_CallMethod_ONEARG(self->style, PyUnicode_FromString("format"), record);
}

PyObject* Formatter_formatStack(Formatter *self, PyObject *stackInfo) {
    // The base implementation just returns the value passed in.
    Py_INCREF(stackInfo);
    return stackInfo;
}

PyObject* Formatter_repr(Formatter *self)
{
    return PyUnicode_FromFormat("<Formatter: fmt='%U'>",
            self->fmt);
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
    {"usesTime", (PyCFunction)Formatter_usesTime, METH_NOARGS, "Return True if the format uses the creation time of the record."},
    {"formatMessage", (PyCFunction)Formatter_formatMessage, METH_O, "Format the message for a record."},
    {"formatStack", (PyCFunction)Formatter_formatStack, METH_O, "Format the stack for a record."},
    {NULL}
};

static PyMemberDef Formatter_members[] = {
    {"_fmt", T_OBJECT_EX, offsetof(Formatter, fmt), 0, "Format string"},
    {"_style", T_OBJECT_EX, offsetof(Formatter, style), 0, "String style formatter"},
    {"datefmt", T_OBJECT_EX, offsetof(Formatter, dateFmt), 0, "Date format string"},
    {NULL}
};

PyTypeObject FormatterType = {
    PyObject_HEAD_INIT(NULL)
    "picologging.Formatter",                    /* tp_name */
    sizeof(Formatter),                          /* tp_basicsize */
    0,                                          /* tp_itemsize */
    (destructor)Formatter_dealloc,              /* tp_dealloc */
    0,                                          /* tp_vectorcall_offset */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_as_async */
    (reprfunc)Formatter_repr,                   /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash */
    0,                                          /* tp_call */
    0,                                          /* tp_str */
    PyObject_GenericGetAttr,                    /* tp_getattro */
    PyObject_GenericSetAttr,                    /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE ,  /* tp_flags */
    PyDoc_STR("Formatter for log records."),    /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    Formatter_methods,                          /* tp_methods */
    Formatter_members,                          /* tp_members */
    0,                                          /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    (initproc)Formatter_init,                   /* tp_init */
    0,                                          /* tp_alloc */
    Formatter_new,                          /* tp_new */
    PyObject_Del,                               /* tp_free */
};