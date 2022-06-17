#include "logger.hxx"
#include "logrecord.hxx"
#include "compat.hxx"
#include <frameobject.h>
#include "picologging.hxx"
#include "filterer.hxx"

int getEffectiveLevel(Logger*self){
    PyObject* logger = (PyObject*)self;
    while (logger != Py_None) {
        // TODO : We could support logging.Logger here through duck-typing..
        // It depends on whether this is requested by the users or not.
        if (!Logger_CheckExact(logger)) {
            PyErr_SetString(PyExc_TypeError, "Logger should be of type picologging.Logger");
            return -1;
        }
        if (((Logger*)logger)->level > 0){
            return ((Logger*)logger)->level;
        }
        logger = ((Logger*)logger)->parent;
        continue;
    }
    return LOG_LEVEL_NOTSET;
}

int Logger_init(Logger *self, PyObject *args, PyObject *kwds)
{
    if (FiltererType.tp_init((PyObject *) self, args, kwds) < 0)
        return -1;

    PyObject *name = NULL;
    unsigned short level = 0;
    static const char *kwlist[] = {"name", "level", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|H", const_cast<char**>(kwlist), &name, &level))
        return -1;
    
    self->name = name;
    Py_INCREF(self->name);
    self->level = level;
    self->parent = Py_None;
    Py_INCREF(self->parent);
    self->propagate = true;
    self->handlers = PyList_New(0);
    Py_INCREF(self->handlers);
    self->disabled = false;
    switch (getEffectiveLevel(self)){
        case LOG_LEVEL_DEBUG:
            self->enabledForDebug = true;
        case LOG_LEVEL_INFO:
            self->enabledForInfo = true;
        case LOG_LEVEL_WARNING:
            self->enabledForWarning = true;
        case LOG_LEVEL_ERROR:
            self->enabledForError = true;
        case LOG_LEVEL_CRITICAL:
            self->enabledForCritical = true;
    }
    self->_const_handle = PyUnicode_FromString("handle");
    self->_const_level = PyUnicode_FromString("level");
    self->_const_unknown = PyUnicode_FromString("<unknown>");
    return 0;
}

PyObject* Logger_dealloc(Logger *self) {
    Py_XDECREF(self->name);
    Py_XDECREF(self->parent);
    Py_XDECREF(self->handlers);
    Py_XDECREF(self->_const_handle);
    Py_XDECREF(self->_const_level);
    Py_XDECREF(self->_const_unknown);
    Py_TYPE(self)->tp_free((PyObject*)self);
    return NULL;
}

PyObject* Logger_repr(Logger *self) {
    return PyUnicode_FromFormat("<Logger '%U' (%d)>", self->name, self->level);
}

PyObject* Logger_setLevel(Logger *self, PyObject *level) {
    if (!PyLong_Check(level)) {
        PyErr_SetString(PyExc_TypeError, "level must be an integer");
        return NULL;
    }
    self->level = (unsigned short)PyLong_AsUnsignedLongMask(level);
    Py_RETURN_NONE;
}

PyObject* Logger_getEffectiveLevel(Logger *self){
    int level = getEffectiveLevel(self);
    if (level == -1)
        return nullptr;
    return PyLong_FromLong(level);
}

LogRecord* Logger_logMessageAsRecord(Logger* self, unsigned short level, PyObject *msg, PyObject *args, PyObject * exc_info, PyObject *extra, PyObject *stack_info, int stacklevel){
    PyFrameObject* frame = PyEval_GetFrame();
    if (frame == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Could not get frame");
        return nullptr;
    }
    PyFrameObject *f = frame->f_back;
    PyFrameObject *orig_f = f;
    while (f != NULL && stacklevel > 1) {
        f = f->f_back;
        stacklevel--;
    }
    if (f == NULL) {
        f = orig_f;
    }
    PyObject *co_filename = f != nullptr ? f->f_code->co_filename : self->_const_unknown;
    PyObject *lineno = f != nullptr ? PyLong_FromLong(f->f_lineno) : PyLong_FromLong(0);
    PyObject *co_name = f != nullptr ? f->f_code->co_name : self->_const_unknown;

    PyObject* record = PyObject_CallFunctionObjArgs(
        (PyObject*)&LogRecordType,
        self->name,
        PyLong_FromUnsignedLong(level),
        co_filename,
        lineno,
        msg,
        args,
        exc_info,
        co_name,
        stack_info,
        NULL
    );
    return (LogRecord*)record;
}

PyObject* Logger_logAndHandle(Logger *self, PyObject *const *args, Py_ssize_t nargs, PyObject *kwds){
    PyObject *msg = args[0];
    PyObject *args_ = PyTuple_New(nargs - 1);
    for (int i = 1; i < nargs; i++) {
        PyTuple_SET_ITEM(args_, i - 1, args[i]);
    }
    LogRecord *record = Logger_logMessageAsRecord(
        self, LOG_LEVEL_DEBUG, msg, args_, /* TODO: Resolve */ Py_None, Py_None, Py_None, 1);

    if (Filterer_filter(&self->filterer, (PyObject*)record) != Py_True)
        Py_RETURN_NONE;
    
    int found = 0;
    Logger* cur = self;
    bool has_parent = true;
    while (has_parent){
        for (int i = 0; i < PyList_GET_SIZE(cur->handlers) ; i++){
            found ++;
            PyObject* handler = PyList_GET_ITEM(cur->handlers, i);
            PyObject* handlerLevel = PyObject_GetAttr(handler, self->_const_level);
            if (handlerLevel == nullptr){
                PyErr_SetString(PyExc_TypeError, "Handler has no level attribute");
                return nullptr;
            }
            
            if (record->levelno >= PyLong_AsLong(handlerLevel)){
                PyObject_CallMethod_ONEARG(handler, self->_const_handle, (PyObject*)record);
            }
            Py_DECREF(handlerLevel);
        }
        if (!cur->propagate || cur->parent == Py_None) {
            has_parent = false;
        } else {
            if (!Logger_CheckExact(cur->parent))
            {
                PyErr_SetString(PyExc_TypeError, "Logger's parent is not an instance of picologging.Logger");
                return nullptr;
            }
            cur = (Logger*)cur->parent;
        }
    }
    // TODO : call last resort handler
    Py_RETURN_NONE;
}

PyObject* Logger_debug(Logger *self, PyObject *const *args, Py_ssize_t nargs, PyObject *kwds) {
    if (self->disabled || !self->enabledForDebug) {
        Py_RETURN_NONE;
    }

    if (nargs < 1){
        PyErr_SetString(PyExc_TypeError, "debug() requires 1 positional argument");
        return nullptr;
    }
    return Logger_logAndHandle(self, args, nargs, kwds);
}

PyObject* Logger_info(Logger *self, PyObject *const *args, Py_ssize_t nargs, PyObject *kwds){
    if (self->disabled || !self->enabledForInfo) {
        Py_RETURN_NONE;
    }

    if (nargs < 1){
        PyErr_SetString(PyExc_TypeError, "info() requires 1 positional argument");
        return nullptr;
    }
    return Logger_logAndHandle(self, args, nargs, kwds);
}
PyObject* Logger_warning(Logger *self, PyObject *const *args, Py_ssize_t nargs, PyObject *kwds){
    if (self->disabled || !self->enabledForWarning) {
        Py_RETURN_NONE;
    }

    if (nargs < 1){
        PyErr_SetString(PyExc_TypeError, "warning() requires 1 positional argument");
        return nullptr;
    }
    return Logger_logAndHandle(self, args, nargs, kwds);
}

PyObject* Logger_fatal(Logger *self, PyObject *const *args, Py_ssize_t nargs, PyObject *kwds){
    return Logger_critical(self, args, nargs, kwds);
}

PyObject* Logger_error(Logger *self, PyObject *const *args, Py_ssize_t nargs, PyObject *kwds){
    if (self->disabled || !self->enabledForError) {
        Py_RETURN_NONE;
    }

    if (nargs < 1){
        PyErr_SetString(PyExc_TypeError, "error() requires 1 positional argument");
        return nullptr;
    }
    return Logger_logAndHandle(self, args, nargs, kwds);
}
PyObject* Logger_critical(Logger *self, PyObject *const *args, Py_ssize_t nargs, PyObject *kwds){
    if (self->disabled || !self->enabledForCritical) {
        Py_RETURN_NONE;
    }

    if (nargs < 1){
        PyErr_SetString(PyExc_TypeError, "critical() requires 1 positional argument");
        return nullptr;
    }
    return Logger_logAndHandle(self, args, nargs, kwds);
}
PyObject* Logger_exception(Logger *self, PyObject *args, PyObject *kwds){}
PyObject* Logger_log(Logger *self, PyObject *args, PyObject *kwds){}

static PyMethodDef Logger_methods[] = {
    {"setLevel", (PyCFunction)Logger_setLevel, METH_O, "Set the level of the logger."},
    {"getEffectiveLevel", (PyCFunction)Logger_getEffectiveLevel, METH_NOARGS, "Get the effective level of the logger."},
    // Logging methods
    {"debug", (PyCFunction)Logger_debug, METH_FASTCALL | METH_KEYWORDS, "Log a message at level DEBUG."},
    {"info", (PyCFunction)Logger_info, METH_FASTCALL | METH_KEYWORDS, "Log a message at level INFO."},
    {"warning", (PyCFunction)Logger_warning, METH_FASTCALL | METH_KEYWORDS, "Log a message at level WARNING."},
    {"error", (PyCFunction)Logger_error, METH_FASTCALL | METH_KEYWORDS, "Log a message at level ERROR."},
    {"critical", (PyCFunction)Logger_critical, METH_FASTCALL | METH_KEYWORDS, "Log a message at level CRITICAL."},
    {"exception", (PyCFunction)Logger_exception, METH_VARARGS, "Log a message at level ERROR."},
    {"fatal", (PyCFunction)Logger_fatal, METH_FASTCALL | METH_KEYWORDS, "Log a message at level FATAL."},
    {"log", (PyCFunction)Logger_log, METH_VARARGS, "Log a message at the specified level."},
    {NULL}
};

static PyMemberDef Logger_members[] = {
    {"name", T_OBJECT_EX, offsetof(Logger, name), 0, "Logger name"},
    {"level", T_USHORT, offsetof(Logger, level), 0, "Logger level"},
    {"parent", T_OBJECT_EX, offsetof(Logger, parent), 0, "Logger parent"},
    {"propagate", T_BOOL, offsetof(Logger, propagate), 0, "Logger propagate"},
    {"handlers", T_OBJECT_EX, offsetof(Logger, handlers), 0, "Logger handlers"},
    {"disabled", T_BOOL, offsetof(Logger, disabled), 0, "Logger disabled"},
    {NULL}
};

PyTypeObject LoggerType = {
    PyObject_HEAD_INIT(NULL)
    "picologging.Logger",                    /* tp_name */
    sizeof(Logger),                          /* tp_basicsize */
    0,                                          /* tp_itemsize */
    (destructor)Logger_dealloc,                /* tp_dealloc */
    0,                                          /* tp_vectorcall_offset */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_as_async */
    (reprfunc)Logger_repr,                      /* tp_repr */
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
    PyDoc_STR("Logging interface."),    /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    Logger_methods,                          /* tp_methods */
    Logger_members,                          /* tp_members */
    0,                                          /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    (initproc)Logger_init,                   /* tp_init */
    0,                                          /* tp_alloc */
    PyType_GenericNew,                          /* tp_new */
    PyObject_Del,                               /* tp_free */
};

