#include <ctime>
#include <charconv>
#include <string_view>
#include "picologging.hxx"
#include "formatter.hxx"
#include "formatstyle.hxx"
#include "logrecord.hxx"

// Size of the temporary buffer on stack to format asctime.
// 64 - is big enough.
// For example: "2024-07-23 03:27:04.982856" - is just 29 bytes
constexpr const size_t MAX_FORMATTED_ASCTIME_SIZE = 64;

PyObject* Formatter_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    Formatter* self = (Formatter*)type->tp_alloc(type, 0);
    if (self != NULL)
    {
        self->fmt = Py_None;
        self->dateFmt = Py_None;
        self->style = Py_None;
        self->_const_line_break = PyUnicode_FromString("\n");
        self->_const_close = PyUnicode_FromString("close");
        self->_const_getvalue = PyUnicode_FromString("getvalue");
        self->_const_usesTime = PyUnicode_FromString("usesTime");
        self->_const_format = PyUnicode_FromString("format");
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
    PyObject * style_c = PyUnicode_FromFormat("%c", style);
    PyObject * styleCls = PyObject_CallFunctionObjArgs(styleType, fmt, Py_None, style_c, NULL);
    Py_DECREF(style_c);
    if (styleCls == nullptr){
        //PyErr_Format(PyExc_ValueError, "Could not initialize Style formatter class.");
        return -1;
    }

    self->style = styleCls;
    self->fmt = Py_NewRef(((FormatStyle*)(self->style))->fmt);
    self->usesTime = (FormatStyle_usesTime((FormatStyle*)self->style) == Py_True);
    self->dateFmt = Py_NewRef(dateFmt);

    self->_dateFmtMicrosendsPos = std::string_view::npos;
    self->_dateFmtStrSize = 0;
    if (dateFmt != Py_None) {
        self->_dateFmtStr = PyUnicode_AsUTF8(self->dateFmt);
        if (self->_dateFmtStr == nullptr) {
            return -1;
        }
        std::string_view dateFmtSV = self->_dateFmtStr;
        self->_dateFmtStrSize = dateFmtSV.size();

        // Later we use temporary buffer allocated on stack to format %f before using standard strftime
        // This check protects against buffer overflow. If dateFmt is too large for the buffer
        // (bigger than in this check) then %f formatting will be disabled thus dateFmt will be passed
        // directly to strftime.
        // -6 means we need extra space for 6 digits of microseconds
        // +2 we reuse 2 digits of %f specifier
        if (self->_dateFmtStrSize <= MAX_FORMATTED_ASCTIME_SIZE - 6 + 2)
            self->_dateFmtMicrosendsPos = dateFmtSV.find("%f");
    } else {
        self->_dateFmtStr = nullptr;
    }

    if (validate){
        if (PyObject_CallMethod(self->style, "validate", NULL) == nullptr){
            Py_CLEAR(self->style);
            Py_CLEAR(self->fmt);
            Py_CLEAR(self->dateFmt);
            return -1;
        }
    }
    return 0;
}

PyObject* Formatter_format(Formatter *self, PyObject *record){
    if (LogRecord_CheckExact(record) || LogRecord_Check(record)){
        LogRecord* logRecord = (LogRecord*)record;
        if (LogRecord_writeMessage(logRecord) == -1){
            return nullptr;
        }
        if (self->usesTime){
            PyObject * asctime = Py_None;
            double createdInt;
            double createdFrac = std::modf(logRecord->created, &createdInt);
            std::time_t created = static_cast<std::time_t>(createdInt);
            std::tm *ct = localtime(&created);

            // Buffer for formatted asctime
            char buf[MAX_FORMATTED_ASCTIME_SIZE + 1];

            if (self->dateFmt != Py_None){
                // dateFmt has been specified for asctime
                size_t len = [&](){
                    if (self->_dateFmtMicrosendsPos != std::string_view::npos){
                        // There is %f in it the provided dateFmt.
                        // Prepare format string for strftime where %f will
                        // be replaced with the actual microseconds
                        char formatStrBuf[MAX_FORMATTED_ASCTIME_SIZE + 1];

                        // Copy everything before %f
                        memcpy(formatStrBuf, self->_dateFmtStr, self->_dateFmtMicrosendsPos);
                        // Format microseconds
                        snprintf(formatStrBuf + self->_dateFmtMicrosendsPos,
                                 sizeof(formatStrBuf) - 1, "%06d",
                                 static_cast<int>(std::round(createdFrac * 1e6)));
                        // Copy everthing after %f, including null terminator
                        memcpy(formatStrBuf + self->_dateFmtMicrosendsPos + 6,
                               self->_dateFmtStr + self->_dateFmtMicrosendsPos + 2,
                               self->_dateFmtStrSize - self->_dateFmtMicrosendsPos - 2 + 1);

                        return strftime(buf, sizeof(buf), formatStrBuf, ct);
                    } else {
                        return strftime(buf, sizeof(buf), self->_dateFmtStr, ct);
                    }
                }();

                asctime = PyUnicode_FromStringAndSize(buf, len);
            } else {
                // dateFmt has not been specified for asctime, use default formatting
                size_t len = strftime(buf, sizeof(buf), "%F %T" , ct);
                len += snprintf(buf + len, sizeof(buf) - len, ",%03d", static_cast<int>(createdFrac * 1e3));
                asctime = PyUnicode_FromStringAndSize(buf, len);
            }

            Py_XDECREF(logRecord->asctime);
            logRecord->asctime = asctime;
        }

        PyObject* result = nullptr;
        if (FormatStyle_CheckExact(self->style)){
            result = FormatStyle_format((FormatStyle*)self->style, record);
        } else {
            result = PyObject_CallMethod_ONEARG(self->style, self->_const_format, record);
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
            PyObject* print_exception = Py_NewRef(PyDict_GetItemString(modDict, "print_exception"));
            PyObject* sio_cls = Py_NewRef(PyDict_GetItemString(modDict, "StringIO"));
            PyObject* sio = PyObject_CallFunctionObjArgs(sio_cls, NULL);
            if (sio == nullptr){
                Py_XDECREF(sio_cls);
                Py_XDECREF(print_exception);
                return nullptr; // Got exception in StringIO.__init__()
            }
            // TODO: Validate length of logRecord->excInfo is >=3
            if (PyObject_CallFunctionObjArgs(
                print_exception,
                PyTuple_GetItem(logRecord->excInfo, 0), 
                PyTuple_GetItem(logRecord->excInfo, 1), 
                PyTuple_GetItem(logRecord->excInfo, 2), 
                Py_None,
                sio,
                NULL) == nullptr)
            {
                Py_XDECREF(sio);
                Py_XDECREF(sio_cls);
                Py_XDECREF(print_exception);
                return nullptr; // Got exception in print_exception()
            }
            PyObject* s = PyObject_CallMethod_NOARGS(sio, self->_const_getvalue);
            if (s == nullptr){
                Py_XDECREF(sio);
                Py_XDECREF(sio_cls);
                Py_XDECREF(print_exception);
                return nullptr; // Got exception in StringIO.getvalue()
            }
            
            if (PyObject_CallMethod_NOARGS(sio, self->_const_close) == nullptr){
                Py_DECREF(s);
                Py_XDECREF(sio);
                Py_XDECREF(sio_cls);
                Py_XDECREF(print_exception);
                return nullptr; // Got exception in StringIO.close()
            }
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
                if (result == nullptr) return nullptr;
            }
            PyUnicode_Append(&result, logRecord->excText);
            if (result == nullptr) return nullptr;
        }
        if (logRecord->stackInfo != Py_None && logRecord->stackInfo != Py_False ) {
            if (PyUnicode_Check(logRecord->stackInfo) ) {
                if (PyUnicode_GET_LENGTH(logRecord->stackInfo) > 0) {
                    if (!PYUNICODE_ENDSWITH(result, self->_const_line_break)) {
                        PyUnicode_Append(&result, self->_const_line_break);
                        if (result == nullptr) return nullptr;
                    }
                    PyUnicode_Append(&result, logRecord->stackInfo);
                    if (result == nullptr) return nullptr;
                }
            } else {
                PyObject* s = PyObject_Str(logRecord->stackInfo);
                if (s == nullptr){
                    return nullptr; // Got exception in str(stackInfo)
                }
                if (!PYUNICODE_ENDSWITH(result, self->_const_line_break)){
                    PyUnicode_Append(&result, self->_const_line_break);
                    if (result == nullptr) return nullptr;
                }
                PyUnicode_Append(&result, s);
                if (result == nullptr) return nullptr;
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
        return PyObject_CallMethod_NOARGS(self->style, self->_const_usesTime);
    }
}

PyObject* Formatter_formatMessage(Formatter *self, PyObject* record){
    return PyObject_CallMethod_ONEARG(self->style, self->_const_format, record);
}

PyObject* Formatter_formatStack(Formatter *self, PyObject *stackInfo) {
    // The base implementation just returns the value passed in.
    return Py_NewRef(stackInfo);
}

PyObject* Formatter_formatException(Formatter *self, PyObject *excInfo) {
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
        PyTuple_GetItem(excInfo, 0), 
        PyTuple_GetItem(excInfo, 1), 
        PyTuple_GetItem(excInfo, 2), 
        Py_None,
        sio,
        NULL) == nullptr)
    {
        Py_XDECREF(sio_cls);
        Py_XDECREF(print_exception);
        return nullptr; // Got exception in print_exception()
    }
    PyObject* s = PyObject_CallMethod_NOARGS(sio, self->_const_getvalue);

    if (s == nullptr){
        Py_XDECREF(sio);
        Py_XDECREF(sio_cls);
        Py_XDECREF(print_exception);
        return nullptr; // Got exception in StringIO.getvalue()
    }
    
    PyObject_CallMethod_NOARGS(sio, self->_const_close);
    Py_DECREF(sio);
    Py_DECREF(sio_cls);
    Py_DECREF(print_exception);
    if (PYUNICODE_ENDSWITH(s, self->_const_line_break)){
        PyObject* s2 = PyUnicode_Substring(s, 0, PyUnicode_GetLength(s) - 1);
        Py_DECREF(s);
        s = s2;
    }
    return s;
}

PyObject* Formatter_repr(Formatter *self)
{
    return PyUnicode_FromFormat("<%s: fmt='%U'>",
            _PyType_Name(Py_TYPE(self)), self->fmt);
}

PyObject* Formatter_dealloc(Formatter *self) {
    Py_CLEAR(self->fmt);
    Py_CLEAR(self->dateFmt);
    Py_CLEAR(self->style);
    Py_CLEAR(self->_const_line_break);
    Py_CLEAR(self->_const_close);
    Py_CLEAR(self->_const_getvalue);
    Py_CLEAR(self->_const_usesTime);
    Py_CLEAR(self->_const_format);
    Py_TYPE(self)->tp_free((PyObject*)self);
    return NULL;
}

static PyMethodDef Formatter_methods[] = {
    {"format", (PyCFunction)Formatter_format, METH_O, "Format record into log event string"},
    {"usesTime", (PyCFunction)Formatter_usesTime, METH_NOARGS, "Return True if the format uses the creation time of the record."},
    {"formatMessage", (PyCFunction)Formatter_formatMessage, METH_O, "Format the message for a record."},
    {"formatStack", (PyCFunction)Formatter_formatStack, METH_O, "Format the stack for a record."},
    {"formatException", (PyCFunction)Formatter_formatException, METH_O, "Format and return the specified exception information as a string."},
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