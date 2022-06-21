#include <thread>
#include <filesystem>
#include "logrecord.hxx"
#include "compat.hxx"
#include "picologging.hxx"

namespace fs = std::filesystem;
#define CACHE_FILEPATH 1
_PyTime_t startTime = current_time();

const FilepathCacheEntry& FilepathCache::lookup(PyObject* pathname){
    /*
     * Notes: A vector ended up being significantly faster than an unordered_map,
     * even though an unordered_map should probably be used.
     * TODO #3 : Cap vector size or decide on a better map type.
     */
    Py_hash_t hash = PyObject_Hash(pathname);
    for (auto& entry : cache){
        if (entry.first == hash){
            return entry.second;
        }
    }
    FilepathCacheEntry* entry = new FilepathCacheEntry();
    fs::path fs_path = fs::path(PyUnicode_AsUTF8(pathname));
#ifdef WIN32
    const wchar_t* filename_wchar = fs_path.filename().c_str();
    const wchar_t* modulename = fs_path.stem().c_str();
    entry->filename = PyUnicode_FromWideChar(filename_wchar, wcslen(filename_wchar)),
    entry->module = PyUnicode_FromWideChar(modulename, wcslen(modulename));
#else
    entry->filename = PyUnicode_FromString(fs_path.filename().c_str());
    entry->module = PyUnicode_FromString(fs_path.stem().c_str());
#endif
    cache.push_back({hash, *entry});
    return *entry;
}

FilepathCache::~FilepathCache(){
    for (auto& entry : cache){
        Py_XDECREF(entry.second.filename);
        Py_XDECREF(entry.second.module);
    }
}

FilepathCache filepathCache = FilepathCache();

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

int LogRecord_init(LogRecord *self, PyObject *initargs, PyObject *kwds)
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
        return -1;

    self->name = name;
    Py_INCREF(name);
    self->msg = msg;
    Py_INCREF(msg);

    // This is a copy of the behaviour in the Python class
    // if (args and len(args) == 1 and isinstance(args[0], collections.abc.Mapping)
    //         and args[0]):
    //         args = args[0]
    Py_ssize_t argsLen = 0;
    if (args != Py_None){
        argsLen = PyObject_Length(args);
    }

    if (argsLen == 1 && PySequence_Check(args)){
        PyObject* firstValue = PySequence_GetItem(args, 0);
        if (PyMapping_Check(firstValue)) {
            args = firstValue;
        }
        Py_DECREF(firstValue);
    }
    if (argsLen == 0) {
        self->hasArgs = false;
    } else {
        self->hasArgs = true;
    }
    self->args = args;
    Py_INCREF(args);

    self->levelno = levelno;
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
    Py_INCREF(levelname);
    self->pathname = pathname;
    Py_INCREF(pathname);

#ifdef CACHE_FILEPATH
    auto filepath = filepathCache.lookup(pathname);
    self->filename = filepath.filename;
    self->module = filepath.module;
#else
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
#endif

    Py_INCREF(self->filename);
    Py_INCREF(self->module);
    self->excInfo = exc_info;
    Py_INCREF(self->excInfo);
    self->excText = Py_None;
    Py_INCREF(self->excText);

    if (sinfo != NULL){
        self->stackInfo = sinfo;
        Py_INCREF(sinfo);
    } else {
        self->stackInfo = Py_None;
        Py_INCREF(Py_None);
    }
    
    self->lineno = lineno;
    if (funcname != NULL){
        self->funcName = funcname;
    } else {
        self->funcName = Py_None;
    }
    Py_INCREF(self->funcName);
    _PyTime_t ctime = current_time();
    if (ctime == -1){
        goto error;
    }

    self->created = _PyTime_AsSecondsDouble(ctime);
    self->msecs = _PyTime_AsMilliseconds(ctime, _PyTime_ROUND_CEILING);
    self->relativeCreated = _PyFloat_FromPyTime((ctime - startTime) * 1000);
    Py_INCREF(self->relativeCreated);
    
    self->thread = PyThread_get_thread_ident(); // Only supported in Python 3.7+, if big demand for 3.6 patch this out for the old API.
    // TODO #2 : See if there is a performant way to get the thread name.
    self->threadName = Py_None;
    Py_INCREF(Py_None);
    // TODO #1 : See if there is a performant way to get the process name.
    self->processName = Py_None;
    Py_INCREF(Py_None);
    self->process = getpid();
    self->message = Py_None;
    Py_INCREF(Py_None);
    self->asctime = Py_None;
    Py_INCREF(Py_None);
    return 0;

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
    return -1;
}

PyObject* LogRecord_dealloc(LogRecord *self)
{
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
    ((PyObject*)self)->ob_type->tp_free((PyObject*)self);
    return nullptr;
}

/**
 * Update the message attribute of the object and return the field
 */
PyObject* LogRecord_getMessage(LogRecord *self)
{
    PyObject *msg = nullptr;
    PyObject *args = self->args;

    if (PyUnicode_Check(self->msg)){
        msg = self->msg;
    } else {
        msg = PyObject_Str(self->msg);
    }

    if (!self->hasArgs) {
        Py_XDECREF(self->message);
        self->message = msg;
        Py_XINCREF(self->message);
    } else {
        Py_XDECREF(self->message);
        self->message = PyUnicode_Format(msg, args);
        Py_XINCREF(self->message);
    }
    return self->message;
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
    PyDict_SetItemString(dict, "levelno", PyLong_FromLong(((LogRecord*)obj)->levelno));
    PyDict_SetItemString(dict, "levelname", ((LogRecord*)obj)->levelname);
    PyDict_SetItemString(dict, "pathname", ((LogRecord*)obj)->pathname);
    PyDict_SetItemString(dict, "filename", ((LogRecord*)obj)->filename);
    PyDict_SetItemString(dict, "module", ((LogRecord*)obj)->module);
    PyDict_SetItemString(dict, "funcName", ((LogRecord*)obj)->funcName);
    PyDict_SetItemString(dict, "lineno", PyLong_FromLong(((LogRecord*)obj)->lineno));
    PyDict_SetItemString(dict, "created", PyFloat_FromDouble(((LogRecord*)obj)->created));
    PyDict_SetItemString(dict, "msecs", PyLong_FromLong(((LogRecord*)obj)->msecs));
    PyDict_SetItemString(dict, "relativeCreated", ((LogRecord*)obj)->relativeCreated);
    PyDict_SetItemString(dict, "thread", PyLong_FromUnsignedLong(((LogRecord*)obj)->thread));
    PyDict_SetItemString(dict, "threadName", ((LogRecord*)obj)->threadName);
    PyDict_SetItemString(dict, "processName", ((LogRecord*)obj)->processName);
    PyDict_SetItemString(dict, "process", PyLong_FromLong(((LogRecord*)obj)->process));
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
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE ,  /* tp_flags */
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
    PyType_GenericNew,                          /* tp_new */
    PyObject_Del,                               /* tp_free */
};