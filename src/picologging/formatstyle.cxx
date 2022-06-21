#include "formatstyle.hxx"
#include "logrecord.hxx"
#include "picologging.hxx"
#include <regex>
#include <cstdarg>

std::regex const fragment_search("\\%\\(\\w+\\)[diouxefgcrsa%]");

FieldMap field_map = {
        {"name", Field_Name},
        {"msg", Field_Msg},
        {"args", Field_Args},
        {"levelno", Field_LevelNo},
        {"levelname", Field_LevelName},
        {"pathname", Field_Pathname},
        {"filename", Field_Filename},
        {"module", Field_Module},
        {"lineno", Field_Lineno},
        {"funcname", Field_FuncName},
        {"created", Field_Created},
        {"msecs", Field_Msecs},
        {"relativeCreated", Field_RelativeCreated},
        {"thread", Field_Thread},
        {"threadName", Field_ThreadName},
        {"processName", Field_ProcessName},
        {"process", Field_Process},
        {"exc_info", Field_ExcInfo},
        {"exc_text", Field_ExcText},
        {"stack_info", Field_StackInfo},
        {"message", Field_Message},
        {"asctime", Field_Asctime},
    };

#define APPEND_STRING(field) \
if (PyUnicode_Check(log_record->field )) { \
    if (_PyUnicodeWriter_WriteStr(&writer, log_record->field ) != 0) { \
        _PyUnicodeWriter_Dealloc(&writer); \
        return nullptr; \
    } \
} else { \
    PyObject* strRepr = PyObject_Str(log_record->field ); \
    if (_PyUnicodeWriter_WriteStr(&writer, strRepr) != 0) { \
        _PyUnicodeWriter_Dealloc(&writer); \
        Py_DECREF(strRepr); \
        return nullptr; \
    } \
    Py_DECREF(strRepr); \
}

#define APPEND_INT(field) {\
    PyObject* field = PyUnicode_FromFormat("%d", log_record->field ); \
    if (_PyUnicodeWriter_WriteStr(&writer, field) != 0) { \
        _PyUnicodeWriter_Dealloc(&writer); \
        Py_DECREF(field); \
        return nullptr; \
    } \
    Py_DECREF(field); }\


int PercentStyle_init(PercentStyle *self, PyObject *args, PyObject *kwds){
    PyObject *fmt = nullptr, *defaults = Py_None;
    static const char *kwlist[] = {"fmt", "defaults", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|O", const_cast<char**>(kwlist), &fmt, &defaults))
        return -1;

    if (!PyUnicode_Check(fmt)){
        PyObject* mod = PICOLOGGING_MODULE(); // borrowed reference
        if (mod == nullptr){
            PyErr_SetString(PyExc_TypeError, "Could not find _picologging module");
            return -1;
        }
        fmt = PyDict_GetItemString(PyModule_GetDict(mod), "default_fmt"); // borrowed reference
        self->usesDefaultFmt = true;
    } else {
        self->usesDefaultFmt = false;
    }
    self->fmt = fmt;
    Py_INCREF(fmt);

    std::string const format_string(PyUnicode_AsUTF8(fmt));
    auto fragments_begin = std::sregex_iterator(format_string.begin(), format_string.end(), fragment_search);
    auto fragments_end = std::sregex_iterator();
    int idx = 0;
    int cursor = 0;
    for (std::sregex_iterator i = fragments_begin; i != fragments_end; ++i) {
            std::smatch match = *i;
            std::string match_str = match.str();
            std::string field_name = match_str.substr(2, match_str.size() - 4);
            if (match.position() != cursor){
                // Add literal fragment
                self->fragments[idx].field = LiteralFragment;
                self->fragments[idx].fragment = PyUnicode_FromString(format_string.substr(cursor, match.position() - cursor).c_str());
                idx ++;
            }
            auto it = field_map.find(field_name);
            if (it != field_map.end()) {
                self->fragments[idx].field = it->second;
                self->fragments[idx].fragment = nullptr;
            } else {
                self->fragments[idx].field = Field_Unknown;
                self->fragments[idx].fragment = PyUnicode_FromString(field_name.c_str());
            }
            cursor = match.position() + match.length();
            idx ++;
        }
    // Add literal fragment at the end if the cursor isn't at the end of the string.
    if (format_string.size() > cursor){
        self->fragments[idx].field = LiteralFragment;
        self->fragments[idx].fragment = PyUnicode_FromString(format_string.substr(cursor, format_string.size() - cursor).c_str());
        idx ++;
    }
    self->defaults = defaults;
    Py_INCREF(defaults);
    return 0;
}

PyObject* PercentStyle_usesTime(PercentStyle *self){
    if (self->usesDefaultFmt)
        Py_RETURN_FALSE;
    int ret = PyUnicode_Find(self->fmt, PyUnicode_FromString("%(asctime)"), 0, PyUnicode_GET_LENGTH(self->fmt), 1);
    if (ret >= 0){
        Py_RETURN_TRUE;
    } else if (ret == -1){
        Py_RETURN_FALSE;
    } else { // -2
        // Encountered error .
        return nullptr;
    }
}

PyObject* PercentStyle_validate(PercentStyle *self){
    /// TODO: #6 #5 Implement percentage style validation.

    return Py_None;
}

PyObject* PercentStyle_format(PercentStyle *self, PyObject *record){
    if (self->defaults == Py_None){
        if (LogRecord_CheckExact(record)){
            _PyUnicodeWriter writer;
            _PyUnicodeWriter_Init(&writer);
            LogRecord* log_record = reinterpret_cast<LogRecord*>(record);
            for (int i = 0 ; i < self->ob_base.ob_size ; i++){
                switch (self->fragments[i].field){
                    /*
                    _PyUnicodeWriter_WriteStr doesn't do any type check (causes segfault)
                    so use the APPEND_STRING macro to use a fast-path if the field is string,
                    otherwise do a PyObject_Str first...
                    TODO: #7 Consider %d, %f format strings..
                    */
                    case Field_Name:
                        APPEND_STRING(name)
                        break;
                    case Field_Msg:
                        APPEND_STRING(msg)
                        break;
                    case Field_Args:
                        APPEND_STRING(args)
                        break;
                    case Field_LevelNo:
                        APPEND_INT(levelno)
                        break;
                    case Field_LevelName:
                        APPEND_STRING(levelname)
                        break;
                    case Field_Pathname:
                        APPEND_STRING(pathname)
                        break;
                    case Field_Filename:
                        APPEND_STRING(filename)
                        break;
                    case Field_Module:
                        APPEND_STRING(module)
                        break;
                    case Field_Lineno:
                        APPEND_INT(lineno)
                        break;
                    case Field_FuncName:
                        APPEND_STRING(funcName)
                        break;
                    case Field_Created: {
                        PyObject *created = PyUnicode_FromFormat("%f", log_record->created);
                        if (_PyUnicodeWriter_WriteStr(&writer, created) != 0) {
                            _PyUnicodeWriter_Dealloc(&writer);
                            Py_DECREF(created);
                            return nullptr;
                        }
                        Py_DECREF(created);
                    }   
                        break;
                    case Field_Msecs:
                        APPEND_INT(msecs)
                        break;
                    case Field_RelativeCreated:
                        APPEND_STRING(relativeCreated)
                        break;
                    case Field_Thread:
                        APPEND_INT(thread)
                        break;
                    case Field_ThreadName:
                        APPEND_STRING(threadName)
                        break;
                    case Field_ProcessName:
                        APPEND_STRING(processName)
                        break;
                    case Field_Process:
                        APPEND_INT(process)
                        break;
                    case Field_ExcInfo:
                        APPEND_STRING(excInfo)
                        break;
                    case Field_ExcText:
                        APPEND_STRING(excText)
                        break;
                    case Field_StackInfo:
                        APPEND_STRING(stackInfo)
                        break;
                    case Field_Message:
                        APPEND_STRING(message)
                        break;
                    case Field_Asctime:
                        APPEND_STRING(asctime)
                        break;
                    case LiteralFragment:
                        if (_PyUnicodeWriter_WriteStr(&writer, self->fragments[i].fragment) != 0) {
                            _PyUnicodeWriter_Dealloc(&writer);
                            return nullptr;
                        }
                        break;
                    case Field_Unknown: {
                        PyObject* attr = PyObject_GetAttr(record, self->fragments[i].fragment);
                        if (attr == nullptr){
                            _PyUnicodeWriter_Dealloc(&writer);
                            return nullptr;
                        }
                        if (_PyUnicodeWriter_WriteStr(&writer, PyObject_Str(attr)) != 0){
                            _PyUnicodeWriter_Dealloc(&writer);
                            Py_DECREF(attr);
                            return nullptr;
                        }
                        Py_DECREF(attr);
                        break;
                    }
                    default:
                        PyErr_SetString(PyExc_ValueError, "Unknown field");
                        _PyUnicodeWriter_Dealloc(&writer);
                        return nullptr;
                }
            }
            return _PyUnicodeWriter_Finish(&writer);
        } else {
            PyObject* recordDict = PyObject_GetAttrString(record, "__dict__");
            if (recordDict == nullptr)
                return nullptr;
            PyObject* result = PyUnicode_Format(self->fmt, recordDict);
            Py_DECREF(recordDict);
            return result;
        }
    }

    PyObject* dict = PyObject_GetAttrString(record, "__dict__");
    if (PyDict_Merge(dict, self->defaults, 1) < 0){
        Py_DECREF(dict);
        return nullptr;
    }
    PyObject* result = PyUnicode_Format(self->fmt, dict);
    Py_DECREF(dict);
    return result;
}

PyObject *
PercentStyle_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyObject *fmt = nullptr, *defaults = Py_None;
    static const char *kwlist[] = {"fmt", "defaults", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|O", const_cast<char**>(kwlist), &fmt, &defaults))
        return NULL;

    int fragmentLen = 0; 
    if (fmt != nullptr && fmt != Py_None && PyUnicode_Check(fmt)){
        std::string const format_string(PyUnicode_AsUTF8(fmt));
        auto fragments_begin = std::sregex_iterator(format_string.begin(), format_string.end(), fragment_search);
        auto fragments_end = std::sregex_iterator();
        int idx = 0;
        int cursor = 0;
        for (std::sregex_iterator i = fragments_begin; i != fragments_end; ++i) {
            std::smatch match = *i;
            // If there is a literal fragment before this one, add it to the list
            if (match.position() != cursor){
                fragmentLen++;
            }
            cursor = match.position() + match.length();
            fragmentLen++;
        }
        // Capture last literal fragment
        if (cursor != format_string.length()){
            fragmentLen++;
        }
    } else {
        // Number of format fragments in DEFAULT_FMT
        fragmentLen = 1;
    }
    PercentStyle* self;
    self = (PercentStyle*)type->tp_alloc(type, fragmentLen);
    if (self){
        Py_SET_SIZE(self, fragmentLen);
    } else {
        PyErr_NoMemory();
        return nullptr;
    }
    return (PyObject*)self;
}

PyObject* PercentStyle_dealloc(PercentStyle *self){
    Py_XDECREF(self->fmt);
    Py_XDECREF(self->defaults);
    for (int i = 0 ; i < self->ob_base.ob_size; i++){
        Py_XDECREF(self->fragments[i].fragment);
    }
    Py_TYPE(self)->tp_free((PyObject*)self);
    return NULL;
}

PyObject* PercentStyle_repr(PercentStyle *self){
    return PyUnicode_FromFormat("<PercentStyle fmt=%s>", self->fmt);
}

static PyMethodDef PercentStyle_methods[] = {
    {"usesTime", (PyCFunction)PercentStyle_usesTime, METH_NOARGS, "Get message"},
    {"validate", (PyCFunction)PercentStyle_validate, METH_NOARGS, "Get message"},
    {"format", (PyCFunction)PercentStyle_format, METH_O, "Get message"},
    {NULL}
};

static PyMemberDef PercentStyle_members[] = {
    {"_fmt", T_OBJECT_EX, offsetof(PercentStyle, fmt), 0, "Format string"},
    {"_defaults", T_OBJECT_EX, offsetof(PercentStyle, defaults), 0, "Default values"},
    {NULL}
};

PyTypeObject PercentStyleType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "picologging.PercentStyle",                 /* tp_name */
    offsetof(PercentStyle, fragments),          /* tp_basicsize */
    sizeof(FormatFragment),                     /* tp_itemsize */
    (destructor)PercentStyle_dealloc,           /* tp_dealloc */
    0,                                          /* tp_vectorcall_offset */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_as_async */
    (reprfunc)PercentStyle_repr,                /* tp_repr */
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
    PyDoc_STR("% formatter for log records."),  /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    PercentStyle_methods,                       /* tp_methods */
    PercentStyle_members,                       /* tp_members */
    0,                                          /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    (initproc)PercentStyle_init,                /* tp_init */
    0,                                          /* tp_alloc */
    PercentStyle_new,                           /* tp_new */
    PyObject_Del,                               /* tp_free */
};