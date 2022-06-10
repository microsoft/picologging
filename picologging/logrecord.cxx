#include <thread>

#include "logrecord.h"

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

PyObject* LogRecord_init(LogRecord *self, PyObject *initargs, PyObject *kwds)
{
    PyObject *name = nullptr, *exc_info = nullptr, *sinfo = nullptr, *msg = nullptr, *args = nullptr, *levelname = nullptr, *pathname = nullptr, *filename = EMPTY_STRING, *module = EMPTY_STRING, *funcname = nullptr;
    int levelno, lineno;
    long msecs;
    static char *kwlist[] = {"name", "level", "pathname", "lineno", "msg", "args", "exc_info", "func", "sinfo", NULL};
    if (!PyArg_ParseTupleAndKeywords(initargs, kwds, "OiOiOOO|OO", kwlist, 
            &name, &levelno, &pathname, &lineno, &msg, &args, &exc_info, &funcname, &sinfo))
        return NULL;
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
            Py_INCREF(CRITICAL);
            break;
        case 40:
            levelname = ERROR;
            Py_INCREF(ERROR);
            break;
        case 30:
            levelname = WARNING;
            Py_INCREF(WARNING);
            break;
        case 20:
            levelname = INFO;
            Py_INCREF(INFO);
            break;
        case 10:
            levelname = DEBUG;
            Py_INCREF(DEBUG);
            break;
        case 0:
            levelname = NOTSET;
            Py_INCREF(NOTSET);
            break;
        default:
            levelname = PyUnicode_FromFormat("%d", levelno);
            break;
    }

    self->levelname = levelname;
    Py_INCREF(levelname);
    self->pathname = pathname;
    Py_INCREF(pathname);
    // TODO : Resolve filename and module name
        self->filename = filename;
        Py_INCREF(filename);
        self->module = module;
        Py_INCREF(module);
    self->excInfo = exc_info;
    Py_INCREF(exc_info);
    self->excText = Py_None;
    Py_INCREF(Py_None);

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

    self->created = _PyFloat_FromPyTime(ctime);
    if (self->created == NULL) {
        //TODO : cleanup refs.
        return NULL;
    }
    Py_INCREF(self->created);

    self->msecs = _PyTime_AsMilliseconds(ctime, _PyTime_ROUND_CEILING);

    self->relativeCreated = _PyFloat_FromPyTime((ctime - startTime) * 1000);
    Py_INCREF(self->relativeCreated);

    // TODO : Implement multi-threading and process support
    self->thread = PyThread_get_thread_ident();
    self->threadName = Py_None;
    Py_INCREF(Py_None);
    self->processName = Py_None;
    Py_INCREF(Py_None);
    self->process = getpid();
    self->message = Py_None;
    Py_INCREF(Py_None);
    self->asctime = Py_None;
    Py_INCREF(Py_None);
    return (PyObject*)self;
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
        return msg;
    } else {
        return PyUnicode_Format(msg, args);
    }
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
    assert(((LogRecord*)obj)->message != nullptr);
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
    .tp_doc = PyDoc_STR("LogRecord objects are used to hold information about log events."),
    .tp_basicsize = sizeof(LogRecord),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc)LogRecord_init,
    .tp_dealloc = (destructor)LogRecord_dealloc,
    .tp_repr = (reprfunc)LogRecord_repr,
    .tp_members = LogRecord_members,
    .tp_methods = LogRecord_methods,
    .tp_getset = LogRecord_getset,
    .tp_dictoffset = offsetof(LogRecord, dict),
    .tp_getattro = PyObject_GenericGetAttr,
    .tp_setattro = PyObject_GenericSetAttr,
};