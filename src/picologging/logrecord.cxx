#include <thread>
#include <filesystem>
#include "logrecord.hxx"
#include "compat.hxx"
#include "picologging.hxx"

namespace fs = std::filesystem;
_PyTime_t startTime = current_time();

static PyObject*
_PyFloat_FromPyTime(_PyTime_t t)
{
    double d = _PyTime_AsSecondsDouble(t);
    return PyFloat_FromDouble(d);
}

_PyTime_t current_time()
{
    _PyTime_t t;
    if (_PyTime_GetSystemClockWithInfo(&t, NULL) < 0) {
        return -1;
    }
    return t;
}

PyObject* LogRecord_new(PyTypeObject* type, PyObject *initargs, PyObject *kwds)
{
    PyObject *name = nullptr, *exc_info = nullptr, *sinfo = nullptr, *msg = nullptr, *args = nullptr, *levelname = nullptr, *pathname = nullptr, *filename = nullptr, *module = nullptr, *funcname = nullptr;
    int levelno, lineno;
    long msecs;
    static const char *kwlist[] = {
        "name",
        "level",
        "pathname",
        "lineno",
        "msg",
        "args",
        "exc_info",
        "func",
        "sinfo",
        NULL};
    if (!PyArg_ParseTupleAndKeywords(initargs, kwds, "OiOiOOO|OO", const_cast<char**>(kwlist), 
            &name, &levelno, &pathname, &lineno, &msg, &args, &exc_info, &funcname, &sinfo))
        return NULL;

    LogRecord* self = (LogRecord*)type->tp_alloc(type, 0);
    if (self == NULL)
    {
        PyErr_NoMemory();
        return NULL;
    }
    return (PyObject*)LogRecord_create(self, name, msg, args, levelno, pathname, lineno, exc_info, funcname, sinfo);
}

LogRecord* LogRecord_create(LogRecord* self, PyObject* name, PyObject* msg, PyObject* args, int levelno, PyObject* pathname, int lineno, PyObject* exc_info, PyObject* funcname, PyObject* sinfo) {
    self->name = Py_NewRef(name);
    self->msg = Py_NewRef(msg);

    // This is a copy of the behaviour in the Python class
    // if (args and len(args) == 1 and isinstance(args[0], collections.abc.Mapping)
    //         and args[0]):
    //         args = args[0]
    Py_ssize_t argsLen = 0;
    if (args != Py_None){
        argsLen = PyObject_Length(args);
    }

    if (argsLen == 1 && PySequence_Check(args)) {
        PyObject* firstValue = PySequence_GetItem(args, 0);
        if (PyDict_Check(firstValue)) {
            args = firstValue;
        }
        Py_DECREF(firstValue);
    }
    if (argsLen == 0) {
        self->hasArgs = false;
    } else {
        self->hasArgs = true;
    }
    self->args = Py_NewRef(args);

    self->levelno = levelno;
    PyObject* levelname = nullptr;
    switch (levelno) {
        case LOG_LEVEL_CRITICAL:
            levelname = PyUnicode_FromString("CRITICAL");
            break;
        case LOG_LEVEL_ERROR:
            levelname = PyUnicode_FromString("ERROR");
            break;
        case LOG_LEVEL_WARNING:
            levelname = PyUnicode_FromString("WARNING");
            break;
        case LOG_LEVEL_INFO:
            levelname = PyUnicode_FromString("INFO");
            break;
        case LOG_LEVEL_DEBUG:
            levelname = PyUnicode_FromString("DEBUG");
            break;
        case LOG_LEVEL_NOTSET:
            levelname = PyUnicode_FromString("NOTSET");
            break;
        default:
            levelname = PyUnicode_FromFormat("%d", levelno);
            break;
    }

    self->levelname = levelname;
    self->pathname = Py_NewRef(pathname);

#ifdef PICOLOGGING_CACHE_FILEPATH
    picologging_state *state = GET_PICOLOGGING_STATE();
    if (state && state->g_filepathCache != nullptr) {
        auto filepath = state->g_filepathCache->lookup(pathname);
        self->filename = Py_NewRef(filepath.filename);
        self->module = Py_NewRef(filepath.module);
    } else {
        // Manual lookup - TODO Raise warning?
        fs::path fs_path = fs::path(PyUnicode_AsUTF8(pathname));
        #ifdef WIN32
            const wchar_t* filename_wchar = fs_path.filename().c_str();
            const wchar_t* modulename = fs_path.stem().c_str();
            self->filename = PyUnicode_FromWideChar(filename_wchar, wcslen(filename_wchar)),
            self->module = PyUnicode_FromWideChar(modulename, wcslen(modulename));
        #else
            self->filename = PyUnicode_FromString(fs_path.filename().c_str());
            self->module = PyUnicode_FromString(fs_path.stem().c_str());
        #endif
    }
#else // PICOLOGGING_CACHE_FILEPATH
    fs::path fs_path = fs::path(PyUnicode_AsUTF8(pathname));
#ifdef WIN32
    const wchar_t* filename_wchar = fs_path.filename().c_str();
    const wchar_t* modulename = fs_path.stem().c_str();
    self->filename = PyUnicode_FromWideChar(filename_wchar, wcslen(filename_wchar)),
    self->module = PyUnicode_FromWideChar(modulename, wcslen(modulename));
#else
    self->filename = PyUnicode_FromString(fs_path.filename().c_str());
    self->module = PyUnicode_FromString(fs_path.stem().c_str());
#endif
#endif // PICOLOGGING_CACHE_FILEPATH

    self->excInfo = Py_NewRef(exc_info);
    self->excText = Py_NewRef(Py_None);

    if (sinfo != NULL){
        self->stackInfo = Py_NewRef(sinfo);
    } else {
        self->stackInfo = Py_NewRef(Py_None);
    }
    
    self->lineno = lineno;
    if (funcname != NULL){
        self->funcName = Py_NewRef(funcname);
    } else {
        self->funcName = Py_NewRef(Py_None);
    }
    _PyTime_t ctime = current_time();
    if (ctime == -1){
        goto error;
    }

    self->created = _PyTime_AsSecondsDouble(ctime);
    self->msecs = _PyTime_AsMilliseconds(ctime, _PyTime_ROUND_CEILING);
    self->relativeCreated = _PyFloat_FromPyTime((ctime - startTime) * 1000);    
    self->thread = PyThread_get_thread_ident(); // Only supported in Python 3.7+, if big demand for 3.6 patch this out for the old API.
    // TODO #2 : See if there is a performant way to get the thread name.
    self->threadName = Py_NewRef(Py_None);
    // TODO #1 : See if there is a performant way to get the process name.
    self->processName = Py_NewRef(Py_None);
    self->process = getpid();
    self->message = Py_NewRef(Py_None);
    self->asctime = Py_NewRef(Py_None);
    return self;

error:
    Py_XDECREF(self->name);
    Py_XDECREF(self->msg);
    Py_XDECREF(self->args);
    Py_XDECREF(self->levelname);
    Py_XDECREF(self->pathname);
    Py_XDECREF(self->filename);
    Py_XDECREF(self->module);
    Py_XDECREF(self->funcName);
    Py_XDECREF(self->relativeCreated);
    Py_XDECREF(self->threadName);
    Py_XDECREF(self->processName);
    Py_XDECREF(self->excInfo);
    Py_XDECREF(self->excText);
    Py_XDECREF(self->stackInfo);
    Py_XDECREF(self->message);
    Py_XDECREF(self->asctime);
    if (!PyErr_Occurred()) {
        PyErr_Format(PyExc_ValueError, "Could not create LogRecord, unknown error.");
    }
    return nullptr;
}

PyObject* LogRecord_dealloc(LogRecord *self)
{
    Py_CLEAR(self->name);
    Py_CLEAR(self->msg);
    Py_CLEAR(self->args);
    Py_CLEAR(self->levelname);
    Py_CLEAR(self->pathname);
    Py_CLEAR(self->filename);
    Py_CLEAR(self->module);
    Py_CLEAR(self->funcName);
    Py_CLEAR(self->relativeCreated);
    Py_CLEAR(self->threadName);
    Py_CLEAR(self->processName);
    Py_CLEAR(self->excInfo);
    Py_CLEAR(self->excText);
    Py_CLEAR(self->stackInfo);
    Py_CLEAR(self->message);
    Py_CLEAR(self->asctime);
    Py_CLEAR(self->dict);
    ((PyObject*)self)->ob_type->tp_free((PyObject*)self);
    return nullptr;
}

int LogRecord_init(LogRecord *self, PyObject *args, PyObject *kwds)
{
    return 0;
}

int LogRecord_writeMessage(LogRecord *self)
{
    PyObject *msg = nullptr;
    PyObject *args = self->args;

    if (PyUnicode_Check(self->msg)){
        // Add new reference for return value, all other code paths return a new object
        msg = Py_NewRef(self->msg);
    } else {
        msg = PyObject_Str(self->msg);
        if (msg == nullptr) {
            return -1;
        }
    }

    if (!self->hasArgs) {
        Py_DECREF(self->message);
        self->message = msg;
        return 0;
    } else {
        PyObject * formatted = PyUnicode_Format(msg, args);
        Py_DECREF(msg);
        if (formatted == nullptr){
            return -1;
        } else {
            Py_DECREF(self->message);
            self->message = formatted;
            return 0;
        }
    }
}

/**
 * Update the message attribute of the object and return the field
 */
PyObject* LogRecord_getMessage(LogRecord *self)
{
    if (LogRecord_writeMessage(self) == -1)
        return nullptr;
    return Py_NewRef(self->message);
}

PyObject* LogRecordLogRecord_getnewargs(LogRecord *self)
{
    return Py_BuildValue("OlOlOOOOO", self->name, self->levelno, self->pathname, self->lineno,  self->msg, self->args, self->excInfo, self->funcName, self->stackInfo);
}

PyObject* LogRecord_repr(LogRecord *self)
{
    return PyUnicode_FromFormat("<LogRecord: %U, %d, %U, %d, '%U'>",
            self->name,
            self->levelno,
            self->pathname,
            self->lineno,
            self->msg);
}

PyObject *
LogRecord_getDict(PyObject *obj, void *context)
{
    PyObject* dict = PyObject_GenericGetDict(obj, context);
    PyDict_SetItemString(dict, "name", ((LogRecord*)obj)->name);
    PyDict_SetItemString(dict, "msg", ((LogRecord*)obj)->msg);
    PyDict_SetItemString(dict, "args", ((LogRecord*)obj)->args);
    
    PyObject * levelno = PyLong_FromLong(((LogRecord*)obj)->levelno);
    PyDict_SetItemString(dict, "levelno", levelno);
     Py_DECREF(levelno);

    PyDict_SetItemString(dict, "levelname", ((LogRecord*)obj)->levelname);
    PyDict_SetItemString(dict, "pathname", ((LogRecord*)obj)->pathname);
    PyDict_SetItemString(dict, "filename", ((LogRecord*)obj)->filename);
    PyDict_SetItemString(dict, "module", ((LogRecord*)obj)->module);
    PyDict_SetItemString(dict, "funcName", ((LogRecord*)obj)->funcName);

    PyObject *lineno = PyLong_FromLong(((LogRecord*)obj)->lineno);
    PyDict_SetItemString(dict, "lineno", lineno); 
    Py_DECREF(lineno);

    PyObject *created = PyFloat_FromDouble(((LogRecord*)obj)->created);
    PyDict_SetItemString(dict, "created", created);
    Py_DECREF(created);

    PyObject *msecs = PyLong_FromLong(((LogRecord*)obj)->msecs);
    PyDict_SetItemString(dict, "msecs", msecs);
    Py_DECREF(msecs);

    PyDict_SetItemString(dict, "relativeCreated", ((LogRecord*)obj)->relativeCreated);

    PyObject *thread = PyLong_FromUnsignedLong(((LogRecord*)obj)->thread);
    PyDict_SetItemString(dict, "thread", thread);
    Py_DECREF(thread);

    PyDict_SetItemString(dict, "threadName", ((LogRecord*)obj)->threadName);
    PyDict_SetItemString(dict, "processName", ((LogRecord*)obj)->processName);

    PyObject *process = PyLong_FromLong(((LogRecord*)obj)->process);
    PyDict_SetItemString(dict, "process", process);
    Py_DECREF(process);

    PyDict_SetItemString(dict, "exc_info", ((LogRecord*)obj)->excInfo);
    PyDict_SetItemString(dict, "exc_text", ((LogRecord*)obj)->excText);
    PyDict_SetItemString(dict, "stack_info", ((LogRecord*)obj)->stackInfo);
    PyDict_SetItemString(dict, "message", ((LogRecord*)obj)->message);
    PyDict_SetItemString(dict, "asctime", ((LogRecord*)obj)->asctime);
    return dict;
}

static PyMemberDef LogRecord_members[] = {
    {"name", T_OBJECT_EX, offsetof(LogRecord, name), 0, "Logger name"},
    {"msg", T_OBJECT_EX, offsetof(LogRecord, msg), 0, "Message (string)"},
    {"args", T_OBJECT_EX, offsetof(LogRecord, args), 0, "Arguments (tuple)"},
    {"levelno", T_INT, offsetof(LogRecord, levelno), 0, "Level number"},
    {"levelname", T_OBJECT_EX, offsetof(LogRecord, levelname), 0, "Level name"},
    {"pathname", T_OBJECT_EX, offsetof(LogRecord, pathname), 0, "File pathname"},
    {"filename", T_OBJECT_EX, offsetof(LogRecord, filename), 0, "File name"},
    {"module", T_OBJECT_EX, offsetof(LogRecord, module), 0, "Module name"},
    {"lineno", T_INT, offsetof(LogRecord, lineno), 0, "Line number"},
    {"funcName", T_OBJECT_EX, offsetof(LogRecord, funcName), 0, "Function name"},
    {"created", T_DOUBLE, offsetof(LogRecord, created), 0, "Created"},
    {"msecs", T_LONG, offsetof(LogRecord, msecs), 0, "Milliseconds"},
    {"relativeCreated", T_OBJECT_EX, offsetof(LogRecord, relativeCreated), 0, "Relative created"},
    {"thread", T_ULONG, offsetof(LogRecord, thread), 0, "Thread"},
    {"threadName", T_OBJECT_EX, offsetof(LogRecord, threadName), 0, "Thread name"},
    {"processName", T_OBJECT_EX, offsetof(LogRecord, processName), 0, "Process name"},
    {"process", T_INT, offsetof(LogRecord, process), 0, "Process"},
    {"exc_info", T_OBJECT_EX, offsetof(LogRecord, excInfo), 0, "Exception info"},
    {"exc_text", T_OBJECT_EX, offsetof(LogRecord, excText), 0, "Exception text"},
    {"stack_info", T_OBJECT_EX, offsetof(LogRecord, stackInfo), 0, "Stack info"},
    {"message", T_OBJECT_EX, offsetof(LogRecord, message), 0, "Message"},
    {"asctime", T_OBJECT_EX, offsetof(LogRecord, asctime), 0, "Asctime"},
    {NULL}
};

static PyMethodDef LogRecord_methods[] = {
    {"getMessage", (PyCFunction)LogRecord_getMessage, METH_NOARGS, "Get message"},
    {"__getnewargs__", (PyCFunction)LogRecordLogRecord_getnewargs, METH_NOARGS, "Picke LogRecord"},
    {NULL}
};

static PyGetSetDef LogRecord_getset[] = {
    {"__dict__", LogRecord_getDict, PyObject_GenericSetDict},
    {NULL}
};

PyTypeObject LogRecordType = {
    PyObject_HEAD_INIT(NULL)
    "picologging.LogRecord",                    /* tp_name */
    sizeof(LogRecord),                          /* tp_basicsize */
    0,                                          /* tp_itemsize */
    (destructor)LogRecord_dealloc,              /* tp_dealloc */
    0,                                          /* tp_vectorcall_offset */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_as_async */
    (reprfunc)LogRecord_repr,                   /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash */
    0,                                          /* tp_call */
    0,                                          /* tp_str */
    PyObject_GenericGetAttr,                    /* tp_getattro */
    PyObject_GenericSetAttr,                    /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,  /* tp_flags */
    PyDoc_STR("LogRecord objects are used to hold information about log events."),  /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    LogRecord_methods,                          /* tp_methods */
    LogRecord_members,                          /* tp_members */
    LogRecord_getset,                           /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    offsetof(LogRecord, dict),                  /* tp_dictoffset */
    (initproc)LogRecord_init,                   /* tp_init */
    0,                                          /* tp_alloc */
    LogRecord_new,                              /* tp_new */
    PyObject_Del,                               /* tp_free */
};