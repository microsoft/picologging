#include "logger.hxx"
#include "logrecord.hxx"
#include "compat.hxx"
#include <frameobject.h>
#include "picologging.hxx"
#include "filterer.hxx"
#include "handler.hxx"

int getEffectiveLevel(Logger*self){
    PyObject* logger = (PyObject*)self;
    while (logger != Py_None) {
        if (!Logger_CheckExact(logger)) {
            PyObject* level = PyObject_GetAttrString(logger, "level");
            if (level == nullptr){
                return -1;
            }
            int result = PyLong_AsLong(level);
            Py_DECREF(level);
            return result;
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
    self->manager = Py_None;
    Py_INCREF(self->manager);
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
    self->_const_exc_info = PyUnicode_FromString("exc_info");
    self->_const_extra = PyUnicode_FromString("extra");
    self->_const_stack_info = PyUnicode_FromString("stack_info");
    self->_fallback_handler = (StreamHandler*)PyObject_CallFunctionObjArgs((PyObject *)&StreamHandlerType, NULL);
    if (self->_fallback_handler == nullptr){
        return -1;
    }
    Py_INCREF(self->_fallback_handler);
    return 0;
}

PyObject* Logger_dealloc(Logger *self) {
    Py_XDECREF(self->name);
    Py_XDECREF(self->parent);
    Py_XDECREF(self->handlers);
    Py_XDECREF(self->manager);
    Py_XDECREF(self->_const_handle);
    Py_XDECREF(self->_const_level);
    Py_XDECREF(self->_const_unknown);
    Py_XDECREF(self->_const_exc_info);
    Py_XDECREF(self->_const_extra);
    Py_XDECREF(self->_const_stack_info);
    Py_XDECREF(self->_fallback_handler);
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
    PyFrameObject *f = PyFrame_GETBACK(frame);
    PyFrameObject *orig_f = f;
    while (f != NULL && stacklevel > 1) {
        f = PyFrame_GETBACK(f);
        stacklevel--;
    }
    if (f == NULL) {
        f = orig_f;
    }
    PyObject *co_filename = f != nullptr ? PyFrame_GETCODE(f)->co_filename : self->_const_unknown;
    PyObject *lineno = f != nullptr ? PyLong_FromLong(PyFrame_GETLINENO(f)) : PyLong_FromLong(0);
    PyObject *co_name = f != nullptr ? PyFrame_GETCODE(f)->co_name : self->_const_unknown;

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

PyObject* Logger_logAndHandle(Logger *self, PyObject *const *args, Py_ssize_t nargs, PyObject *kwds, unsigned short level){
    PyObject *msg = args[0];
    PyObject *args_ = PyTuple_New(nargs - 1);
    for (int i = 1; i < nargs; i++) {
        PyTuple_SET_ITEM(args_, i - 1, args[i]);
    }
    PyObject* exc_info = kwds != nullptr ? PyDict_GetItem(kwds, self->_const_exc_info) : nullptr;
    if (exc_info == nullptr){
        exc_info = Py_None;
    } else {
        if (PyExceptionClass_Check(exc_info)){
            // TODO : Add references to tuple items.
            PyObject * unpackedExcInfo = PyTuple_New(3);
            PyTuple_SET_ITEM(unpackedExcInfo, 0, (PyObject*)Py_TYPE(exc_info));
            PyTuple_SET_ITEM(unpackedExcInfo, 1, exc_info);
            PyTuple_SET_ITEM(unpackedExcInfo, 2, PyObject_GetAttrString(exc_info, "__traceback__"));
            exc_info = unpackedExcInfo;
        } else if (!PyTuple_CheckExact(exc_info)){ // Probably Py_TRUE
            PyObject * unpackedExcInfo = PyTuple_New(3);
            PyErr_GetExcInfo(&PyTuple_GET_ITEM(unpackedExcInfo, 0), &PyTuple_GET_ITEM(unpackedExcInfo, 1), &PyTuple_GET_ITEM(unpackedExcInfo, 2));
            exc_info = unpackedExcInfo;
        }
    }
    PyObject* extra = kwds != nullptr ? PyDict_GetItem(kwds, self->_const_extra) : nullptr;
    if (extra == nullptr){
        extra = Py_None;
    }
    PyObject* stack_info = kwds != nullptr ? PyDict_GetItem(kwds, self->_const_stack_info) : nullptr;
    if (stack_info == nullptr){
        stack_info = Py_False;
    }
    LogRecord *record = Logger_logMessageAsRecord(
        self, level, msg, args_, exc_info, extra, stack_info, 1);

    if (Filterer_filter(&self->filterer, (PyObject*)record) != Py_True)
        Py_RETURN_NONE;
    
    int found = 0;
    Logger* cur = self;
    bool has_parent = true;
    while (has_parent){
        for (int i = 0; i < PyList_GET_SIZE(cur->handlers) ; i++){
            found ++;
            PyObject* handler = PyList_GET_ITEM(cur->handlers, i);
            if (Handler_Check(handler)){
                if (record->levelno >= ((Handler*)handler)->level){
                    if (Handler_handle((Handler*)handler, (PyObject*)record) == nullptr){
                        return nullptr;
                    }
                }
            } else {
                PyObject* handlerLevel = PyObject_GetAttr(handler, self->_const_level);
                if (handlerLevel == nullptr){
                    PyErr_SetString(PyExc_TypeError, "Handler has no level attribute");
                    return nullptr;
                }
                
                if (record->levelno >= PyLong_AsLong(handlerLevel)){
                    if (PyObject_CallMethod_ONEARG(handler, self->_const_handle, (PyObject*)record) == nullptr){
                        Py_DECREF(handlerLevel);
                        return nullptr;
                    }
                }
                Py_DECREF(handlerLevel);
            }
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
    if (found == 0){
        if (record->levelno >= ((Handler*)self->_fallback_handler)->level){
            if (Handler_handle((Handler*)self->_fallback_handler, (PyObject*)record) == nullptr){
                return nullptr;
            }
        }
    }
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
    return Logger_logAndHandle(self, args, nargs, kwds, LOG_LEVEL_DEBUG);
}

PyObject* Logger_info(Logger *self, PyObject *const *args, Py_ssize_t nargs, PyObject *kwds){
    if (self->disabled || !self->enabledForInfo) {
        Py_RETURN_NONE;
    }

    if (nargs < 1){
        PyErr_SetString(PyExc_TypeError, "info() requires 1 positional argument");
        return nullptr;
    }
    return Logger_logAndHandle(self, args, nargs, kwds, LOG_LEVEL_INFO);
}
PyObject* Logger_warning(Logger *self, PyObject *const *args, Py_ssize_t nargs, PyObject *kwds){
    if (self->disabled || !self->enabledForWarning) {
        Py_RETURN_NONE;
    }

    if (nargs < 1){
        PyErr_SetString(PyExc_TypeError, "warning() requires 1 positional argument");
        return nullptr;
    }
    return Logger_logAndHandle(self, args, nargs, kwds, LOG_LEVEL_WARNING);
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
    return Logger_logAndHandle(self, args, nargs, kwds, LOG_LEVEL_ERROR);
}

PyObject* Logger_critical(Logger *self, PyObject *const *args, Py_ssize_t nargs, PyObject *kwds){
    if (self->disabled || !self->enabledForCritical) {
        Py_RETURN_NONE;
    }

    if (nargs < 1){
        PyErr_SetString(PyExc_TypeError, "critical() requires 1 positional argument");
        return nullptr;
    }
    return Logger_logAndHandle(self, args, nargs, kwds, LOG_LEVEL_CRITICAL);
}

PyObject* Logger_exception(Logger *self, PyObject *const *args, Py_ssize_t nargs, PyObject *kwds){
    if (kwds == nullptr){
        kwds = PyDict_New();
    }
    PyDict_SetItemString(kwds, "exc_info", Py_True);
    return Logger_logAndHandle(self, args, nargs, kwds, LOG_LEVEL_ERROR);
}

PyObject* Logger_log(Logger *self, PyObject *const *args, Py_ssize_t nargs, PyObject *kwds){
    if (nargs < 2){
        PyErr_SetString(PyExc_TypeError, "log() requires 2 positional arguments");
        return nullptr;
    }
    PyObject *level = args[0];
    unsigned short levelno = PyLong_AsUnsignedLongMask(level);
    if (PyErr_Occurred()){
        PyErr_SetString(PyExc_TypeError, "level must be an integer");
        return nullptr;
    }
    // Pop the level off the args and send the new tuple to the logAndHandle method
    PyObject *args_ = PyTuple_New(nargs - 1);
    for (int i = 1; i < nargs; i++) {
        PyTuple_SET_ITEM(args_, i - 1, args[i]);
    }
    return Logger_logAndHandle(self, ((PyTupleObject*)args_)->ob_item, nargs - 1, kwds, levelno);
}

PyObject* Logger_addHandler(Logger *self, PyObject *handler) {
    if (PySequence_Contains(self->handlers, handler)) {
        Py_RETURN_NONE;
    }
    PyList_Append(self->handlers, handler);
    Py_RETURN_NONE;
}

PyObject* Logger_removeHandler(Logger *self, PyObject *handler) {
    if (PySequence_Contains(self->handlers, handler)) {
        return PyObject_CallMethod_ONEARG(self->handlers, PyUnicode_FromString("remove"), handler);
    }
    Py_RETURN_NONE;
}

static PyMethodDef Logger_methods[] = {
    {"setLevel", (PyCFunction)Logger_setLevel, METH_O, "Set the level of the logger."},
    {"getEffectiveLevel", (PyCFunction)Logger_getEffectiveLevel, METH_NOARGS, "Get the effective level of the logger."},
    {"addHandler", (PyCFunction)Logger_addHandler, METH_O, "Add a handler to the logger."},
    {"removeHandler", (PyCFunction)Logger_removeHandler, METH_O, "Remove a handler from the logger."},
    // Logging methods
    {"debug", (PyCFunction)Logger_debug, METH_FASTCALL | METH_KEYWORDS, "Log a message at level DEBUG."},
    {"info", (PyCFunction)Logger_info, METH_FASTCALL | METH_KEYWORDS, "Log a message at level INFO."},
    {"warning", (PyCFunction)Logger_warning, METH_FASTCALL | METH_KEYWORDS, "Log a message at level WARNING."},
    {"error", (PyCFunction)Logger_error, METH_FASTCALL | METH_KEYWORDS, "Log a message at level ERROR."},
    {"critical", (PyCFunction)Logger_critical, METH_FASTCALL | METH_KEYWORDS, "Log a message at level CRITICAL."},
    {"exception", (PyCFunction)Logger_exception, METH_FASTCALL | METH_KEYWORDS, "Log a message at level ERROR."},
    {"fatal", (PyCFunction)Logger_fatal, METH_FASTCALL | METH_KEYWORDS, "Log a message at level FATAL."},
    {"log", (PyCFunction)Logger_log, METH_FASTCALL | METH_KEYWORDS, "Log a message at the specified level."},
    {NULL}
};

static PyMemberDef Logger_members[] = {
    {"name", T_OBJECT_EX, offsetof(Logger, name), 0, "Logger name"},
    {"level", T_USHORT, offsetof(Logger, level), 0, "Logger level"},
    {"parent", T_OBJECT_EX, offsetof(Logger, parent), 0, "Logger parent"},
    {"propagate", T_BOOL, offsetof(Logger, propagate), 0, "Logger propagate"},
    {"handlers", T_OBJECT_EX, offsetof(Logger, handlers), 0, "Logger handlers"},
    {"disabled", T_BOOL, offsetof(Logger, disabled), 0, "Logger disabled"},
    {"manager", T_OBJECT_EX, offsetof(Logger, manager), 0, "Logger manager"},
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

