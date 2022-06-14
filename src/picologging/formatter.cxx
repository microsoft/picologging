#include "formatter.hxx"
#include "formatstyle.hxx"
#include "logrecord.hxx"



int Formatter_init(Formatter *self, PyObject *args, PyObject *kwds){
    PyObject *fmt = Py_None, *dateFmt = Py_None;
    char style = '%';
    bool validate = true;
    static const char *kwlist[] = {"fmt", "datefmt", "style", "validate", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OOCp", const_cast<char**>(kwlist), &fmt, &dateFmt, &style, &validate))
        return -1;

    PyObject* styleType = nullptr;

    switch (style){
        case '%':
            /* Call the class object. */
            styleType = (PyObject*)&PercentStyleType;
            break;
        default:
            PyErr_SetString(PyExc_ValueError, "Unsupported style");
            return -1;
    }

    PyObject * styleCls = PyObject_CallFunctionObjArgs(styleType, fmt, NULL);
    if (PyErr_Occurred()){ // Got exception in PercentStyle.__init__()
        return -1;
    }
    if (styleCls == nullptr){
        PyErr_Format(PyExc_ValueError, "Could not initialize Style formatter class.");
        return -1;
    }

    self->style = styleCls;
    Py_INCREF(self->style);

    self->fmt = ((PercentStyle*)(self->style))->fmt;
    Py_INCREF(fmt);

    self->dateFmt = dateFmt;
    Py_INCREF(dateFmt);

    if (validate){
        if (PyObject_CallMethod(self->style, "validate", NULL) == nullptr){
            Py_DECREF(self->style);
            Py_DECREF(fmt);
            Py_DECREF(dateFmt);
            return -1;
        }
    }

    return 0;
}

PyObject* Formatter_format(Formatter *self, PyObject *record){
    if (LogRecord_CheckExact(record)){
        LogRecord* logRecord = (LogRecord*)record;
        LogRecord_getMessage(logRecord);
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
    PyObject_HEAD_INIT(NULL)
    "picologging.Formatter",                    /* tp_name */
    sizeof(Formatter),                          /* tp_basicsize */
    0,                                          /* tp_itemsize */
    (destructor)Formatter_dealloc,              /* tp_dealloc */
    0,                                          /* tp_vectorcall_offset */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_as_async */
    (reprfunc)PyObject_Repr,                   /* tp_repr */
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
    PyType_GenericNew,                          /* tp_new */
    PyObject_Del,                               /* tp_free */
};