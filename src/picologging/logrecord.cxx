#include <thread>
#include <filesystem>
#include "logrecord.hxx"

namespace fs = std::filesystem;

_PyTime_t startTime = current_time();

static PyObject* CRITICAL = PyUnicode_FromString("CRITICAL");
static PyObject* ERROR = PyUnicode_FromString("ERROR");
static PyObject* WARNING = PyUnicode_FromString("WARNING");
static PyObject* INFO = PyUnicode_FromString("INFO");
static PyObject* DEBUG = PyUnicode_FromString("DEBUG");
static PyObject* NOTSET = PyUnicode_FromString("NOTSET");
static PyObject* EMPTY_STRING = PyUnicode_FromString("");

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
    static char *kwlist[] = {
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
    if (!PyArg_ParseTupleAndKeywords(initargs, kwds, "OiOiOOO|OO", kwlist, 
            &name, &levelno, &pathname, &lineno, &msg, &args, &exc_info, &funcname, &sinfo))
        return -1;
    self->name = name;
    Py_INCREF(name);
    self->msg = msg;
    Py_INCREF(msg);

    // TODO :
    // if (args and len(args) == 1 and isinstance(args[0], collections.abc.Mapping)
    //         and args[0]):
    //         args = args[0]
    if (args == Py_None){
        self->hasArgs = false;
    } else if (PySequence_Check(args) && PySequence_Length(args) == 0) {
        self->hasArgs = false;
    } else {
        self->hasArgs = true;
    }
    self->args = args;
    Py_INCREF(args);

    self->levelno = levelno;
    switch (levelno) {
        case 50:
            levelname = CRITICAL;
            break;
        case 40:
            levelname = ERROR;
            break;
        case 30:
            levelname = WARNING;
            break;
        case 20:
            levelname = INFO;
            break;
        case 10:
            levelname = DEBUG;
            break;
        case 0:
            levelname = NOTSET;
            break;
        default:
            levelname = PyUnicode_FromFormat("%d", levelno);
            break;
    }

    self->levelname = levelname;
    Py_INCREF(levelname);
    self->pathname = pathname;
    Py_INCREF(pathname);

    fs::path fs_path = fs::path(PyUnicode_AsUTF8(pathname));
    self->filename = PyUnicode_FromString(fs_path.filename().c_str());
    Py_INCREF(self->filename);
    self->module = PyUnicode_FromString(fs_path.stem().c_str());
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
        Py_INCREF(funcname);
    } else {
        self->funcName = EMPTY_STRING;
        Py_INCREF(EMPTY_STRING);
    }
    _PyTime_t ctime = current_time();
    if (ctime == -1){
        Py_DECREF(self->funcName);
        Py_DECREF(self->stackInfo);
        Py_DECREF(self->excText);
        Py_DECREF(self->excInfo);
        Py_DECREF(self->module);
        Py_DECREF(self->filename);
        Py_DECREF(self->pathname);
        Py_DECREF(self->levelname);
        Py_DECREF(self->msg);
        Py_DECREF(self->args);
        Py_DECREF(self->name);
        if (!PyErr_Occurred()) {
            PyErr_Format(PyExc_EnvironmentError, "Could not get current time,");
        }
        return -1;
    }

    self->created = _PyFloat_FromPyTime(ctime);
    if (self->created == NULL) {
        Py_DECREF(self->funcName);
        Py_DECREF(self->stackInfo);
        Py_DECREF(self->excText);
        Py_DECREF(self->excInfo);
        Py_DECREF(self->module);
        Py_DECREF(self->filename);
        Py_DECREF(self->pathname);
        Py_DECREF(self->levelname);
        Py_DECREF(self->msg);
        Py_DECREF(self->args);
        Py_DECREF(self->name);
        if (!PyErr_Occurred()) {
            PyErr_Format(PyExc_EnvironmentError, "Could not get current time,");
        }
        return -1;
    }
    Py_INCREF(self->created);

    self->msecs = _PyTime_AsMilliseconds(ctime, _PyTime_ROUND_CEILING);

    self->relativeCreated = _PyFloat_FromPyTime((ctime - startTime) * 1000);
    Py_INCREF(self->relativeCreated);

    // TODO : Implement multi-threading and process support
    self->thread = PyThread_get_thread_ident(); // Only supported in Python 3.7+, if big demand for 3.6 patch this out for the old API.
    self->threadName = Py_None;
    Py_INCREF(Py_None);
    self->processName = Py_None;
    Py_INCREF(Py_None);
    self->process = getpid();
    self->message = Py_None;
    Py_INCREF(Py_None);
    self->asctime = Py_None;
    Py_INCREF(Py_None);
    return 0;
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
    Py_XDECREF(self->created);
    Py_XDECREF(self->relativeCreated);
    Py_XDECREF(self->threadName);
    Py_XDECREF(self->processName);
    Py_XDECREF(self->excInfo);
    Py_XDECREF(self->excText);
    Py_XDECREF(self->stackInfo);
    Py_XDECREF(self->message);
    Py_XDECREF(self->asctime);
    ((PyObject*)self)->ob_type->tp_free((PyObject*)self);
    return NULL;
}

/**
 * Update the message attribute of the object and return the field
 */
PyObject* LogRecord_getMessage(LogRecord *self)
{
    PyObject *msg = NULL;
    PyObject *args = self->args;
    PyObject *result = NULL;

    if (PyUnicode_Check(self->msg)){
        msg = self->msg;
    } else {
        msg = PyObject_Str(self->msg);
    }

    if (!self->hasArgs) {
        Py_DECREF(self->message);
        self->message = msg;
        Py_INCREF(self->message);
    } else {
        Py_DECREF(self->message);
        self->message = PyUnicode_Format(msg, args);
        Py_INCREF(self->message);
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
    PyDict_SetItemString(dict, "created", ((LogRecord*)obj)->created);
    PyDict_SetItemString(dict, "msecs", PyLong_FromLong(((LogRecord*)obj)->msecs));
    PyDict_SetItemString(dict, "relativeCreated", ((LogRecord*)obj)->relativeCreated);
    PyDict_SetItemString(dict, "thread", PyLong_FromLong(((LogRecord*)obj)->thread));
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
    {"created", T_OBJECT_EX, offsetof(LogRecord, created), 0, "Created"},
    {"msecs", T_LONG, offsetof(LogRecord, msecs), 0, "Milliseconds"},
    {"relativeCreated", T_OBJECT_EX, offsetof(LogRecord, relativeCreated), 0, "Relative created"},
    {"thread", T_INT, offsetof(LogRecord, thread), 0, "Thread"},
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
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "picologging.LogRecord",
    .tp_basicsize = sizeof(LogRecord),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)LogRecord_dealloc,
    .tp_repr = (reprfunc)LogRecord_repr,
    .tp_getattro = PyObject_GenericGetAttr,
    .tp_setattro = PyObject_GenericSetAttr,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = PyDoc_STR("LogRecord objects are used to hold information about log events."),
    .tp_methods = LogRecord_methods,
    .tp_members = LogRecord_members,
    .tp_getset = LogRecord_getset,
    .tp_dictoffset = offsetof(LogRecord, dict),
    .tp_init = (initproc)LogRecord_init,
    .tp_new = PyType_GenericNew,
};