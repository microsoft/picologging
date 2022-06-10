#include "formatstyle.h"
#include "logrecord.h"

#include <regex>
#include <cstdarg>

static PyObject* DEFAULT_FMT = PyUnicode_FromString("%(message)s");
static PyObject* ASCTIME_SEARCH = PyUnicode_FromString("%(asctime)");
std::regex const fragment_search("\\%\\(\\w+\\)[diouxefgcrsa%]");

FieldMap field_map = {
        {"name", Field_Name},
        {"msg", Field_Msg},
        // {"asctime", Field_Asctime},
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
    };

#define APPEND_STRING(field) \
if (PyUnicode_Check(log_record->field )) { \ 
    if (_PyUnicodeWriter_WriteStr(&writer, log_record->field ) != 0) { \ 
        _PyUnicodeWriter_Dealloc(&writer); \
        return nullptr; \
    } \
} else { \
    if (_PyUnicodeWriter_WriteStr(&writer, PyObject_Str(log_record->field )) != 0) { \
        _PyUnicodeWriter_Dealloc(&writer); \
        return nullptr; \
    } \
}

PyObject* PercentStyle_init(PercentStyle *self, PyObject *args, PyObject *kwds){
    PyObject *fmt = nullptr, *defaults = Py_None;
    static char *kwlist[] = {"fmt", "defaults", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|O", kwlist, &fmt, &defaults))
        return NULL;

    if (!PyUnicode_Check(fmt)){
        fmt = DEFAULT_FMT;
        self->usesDefaultFmt = true;
    } else {
        self->usesDefaultFmt = false;
    }
    self->fmt = fmt;
    Py_IncRef(fmt);

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
    Py_IncRef(defaults);
    return (PyObject*)self;
}

PyObject* PercentStyle_usesTime(PercentStyle *self){
    if (self->usesDefaultFmt)
        Py_RETURN_FALSE;
    int ret = PyUnicode_Find(self->fmt, ASCTIME_SEARCH , 0, PyUnicode_GET_LENGTH(self->fmt), 1);
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
    /// TODO
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
                    TODO: Consider %d, %f format strings..
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
                        if (_PyUnicodeWriter_WriteStr(&writer, PyUnicode_FromFormat("%d", log_record->levelno)) != 0) {
                            _PyUnicodeWriter_Dealloc(&writer);
                            return nullptr;
                        }
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
                        if (_PyUnicodeWriter_WriteStr(&writer, PyUnicode_FromFormat("%d", log_record->lineno)) != 0) {
                            _PyUnicodeWriter_Dealloc(&writer);
                            return nullptr;
                        }
                        break;
                    case Field_FuncName:
                        APPEND_STRING(funcName)
                        break;
                    case Field_Created:
                        APPEND_STRING(created)
                        break;
                    case Field_Msecs:
                        if (_PyUnicodeWriter_WriteStr(&writer, PyUnicode_FromFormat("%d", log_record->msecs)) != 0) {
                            _PyUnicodeWriter_Dealloc(&writer);
                            return nullptr;
                        }
                        break;
                    case Field_RelativeCreated:
                        APPEND_STRING(relativeCreated)
                        break;
                    case Field_Thread:
                        if (_PyUnicodeWriter_WriteStr(&writer, PyUnicode_FromFormat("%d", log_record->thread)) != 0) {
                            _PyUnicodeWriter_Dealloc(&writer);
                            return nullptr;
                        }
                        break;
                    case Field_ThreadName:
                        APPEND_STRING(threadName)
                        break;
                    case Field_ProcessName:
                        APPEND_STRING(processName)
                        break;
                    case Field_Process:
                        if (_PyUnicodeWriter_WriteStr(&writer, PyUnicode_FromFormat("%d", log_record->process)) != 0) {
                            _PyUnicodeWriter_Dealloc(&writer);
                            return nullptr;
                        }
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
                        if (_PyUnicodeWriter_WriteStr(&writer, PyObject_Str(PyObject_GetAttr(record, self->fragments[i].fragment))) != 0){
                            _PyUnicodeWriter_Dealloc(&writer);
                            return nullptr;
                        }
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
            return PyUnicode_Format(self->fmt, PyObject_GetAttrString(record, "__dict__"));
        }
    }

    PyObject* dict = PyObject_GetAttrString(record, "__dict__");
    if (PyDict_Merge(dict, self->defaults, 1) < 0){
        Py_DECREF(dict);
        return nullptr;
    }
    return PyUnicode_Format(self->fmt, dict);
}

PyObject *
PercentStyle_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyObject *fmt = nullptr, *defaults = Py_None;
    static char *kwlist[] = {"fmt", "defaults", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|O", kwlist, &fmt, &defaults))
        return NULL;

    ssize_t fragmentLen = 0; 
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
    if (self != nullptr){
        self->ob_base.ob_size = fragmentLen;
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
    .tp_name = "picologging.PercentStyle",
    .tp_doc = PyDoc_STR("% formatter for log records."),
    .tp_basicsize = offsetof(PercentStyle, fragments),
    .tp_itemsize = sizeof(FormatFragment),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PercentStyle_new,
    .tp_init = (initproc)PercentStyle_init,
    .tp_dealloc = (destructor)PercentStyle_dealloc,
    .tp_repr = (reprfunc)PyObject_Repr,
    .tp_members = PercentStyle_members,
    .tp_methods = PercentStyle_methods,
    .tp_getattro = PyObject_GenericGetAttr,
    .tp_setattro = PyObject_GenericSetAttr,
};
