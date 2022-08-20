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

PyObject* Logger_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    Logger* self = (Logger*)FiltererType.tp_new(type, args, kwds);
    if (self != NULL)
    {
        self->name = Py_None;
        Py_INCREF(self->name);
        self->parent = Py_None;
        Py_INCREF(self->parent);
        self->propagate = true;
        self->handlers = PyList_New(0);
        if (self->handlers == NULL){
            Py_DECREF(self->parent);
            return nullptr;
        }
        self->disabled = false;
        self->manager = Py_None;
        Py_INCREF(self->manager);
        
        self->_fallback_handler = (StreamHandler*)PyObject_CallFunctionObjArgs((PyObject *)&StreamHandlerType, NULL);
        if (self->_fallback_handler == nullptr){
            Py_DECREF(self->parent);
            Py_DECREF(self->handlers);
            Py_DECREF(self->manager);
            return nullptr;
        }
        Py_INCREF(self->_fallback_handler);
        self->_const_handle = PyUnicode_FromString("handle");
        self->_const_level = PyUnicode_FromString("level");
        self->_const_unknown = PyUnicode_FromString("<unknown>");
        self->_const_exc_info = PyUnicode_FromString("exc_info");
        self->_const_extra = PyUnicode_FromString("extra");
        self->_const_stack_info = PyUnicode_FromString("stack_info");
        self->_const_line_break = PyUnicode_FromString("\n");
    }
    return (PyObject*)self;
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

    self->enabledForDebug = false;
    self->enabledForInfo = false;
    self->enabledForWarning = false;
    self->enabledForError = false;
    self->enabledForCritical = false;
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
    Py_XDECREF(self->_const_line_break);
    Py_XDECREF(self->_fallback_handler);
    Py_TYPE(self)->tp_free((PyObject*)self);
    return NULL;
}

PyObject* Logger_repr(Logger *self) {
    std::string level = _getLevelName(getEffectiveLevel(self));
    return PyUnicode_FromFormat("<Logger '%U' (%s)>", self->name, level.c_str());
}

PyObject* Logger_setLevel(Logger *self, PyObject *level) {
    if (!PyLong_Check(level)) {
        PyErr_SetString(PyExc_TypeError, "level must be an integer");
        return NULL;
    }
    self->level = (unsigned short)PyLong_AsUnsignedLongMask(level);

    self->enabledForDebug = false;
    self->enabledForInfo = false;
    self->enabledForWarning = false;
    self->enabledForError = false;
    self->enabledForCritical = false;
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
    Py_RETURN_NONE;
    // TODO: Should reset parent/child loggers
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

    if (stack_info == Py_True){
        PyObject* mod = PICOLOGGING_MODULE(); // borrowed reference
        PyObject* modDict = PyModule_GetDict(mod); // borrowed reference
        PyObject* print_stack = PyDict_GetItemString(modDict, "print_stack"); // PyDict_GetItemString returns a borrowed reference
        if (print_stack == nullptr){
            PyErr_SetString(PyExc_RuntimeError, "Could not get print_stack");
            return nullptr;
        }
        Py_XINCREF(print_stack);
        PyObject* sio_cls = PyDict_GetItemString(modDict, "StringIO");
        Py_XINCREF(sio_cls);
        PyObject* sio = PyObject_CallFunctionObjArgs(sio_cls, NULL);
        if (sio == nullptr){
            Py_XDECREF(sio_cls);
            Py_XDECREF(print_stack);
            return nullptr; // Got exception in StringIO.__init__()
        }
        PyObject* printStackResult = PyObject_CallFunctionObjArgs(
            print_stack,
            Py_None,
            Py_None,
            sio,
            NULL);
        if (printStackResult == nullptr)
        {
            Py_XDECREF(sio_cls);
            Py_XDECREF(print_stack);
            return nullptr; // Got exception in print_stack()
        }
        Py_DECREF(printStackResult);
        PyObject* s = PyObject_CallMethod_NOARGS(sio, PyUnicode_FromString("getvalue"));
        if (s == nullptr){
            Py_XDECREF(sio);
            Py_XDECREF(sio_cls);
            Py_XDECREF(print_stack);
            return nullptr; // Got exception in StringIO.getvalue()
        }
        
        Py_XDECREF(PyObject_CallMethod_NOARGS(sio, PyUnicode_FromString("close")));
        Py_DECREF(sio);
        Py_DECREF(sio_cls);
        Py_DECREF(print_stack);
        if (PYUNICODE_ENDSWITH(s, self->_const_line_break)){
            PyObject* s2 = PyUnicode_Substring(s, 0, PyUnicode_GetLength(s) - 1);
            Py_DECREF(s);
            s = s2;
        }
        stack_info = s;
    }

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
    Py_DECREF(lineno);

    return (LogRecord*)record;
}

PyObject* Logger_logAndHandle(Logger *self, PyObject *args, PyObject *kwds, unsigned short level){
    PyObject *msg = PyTuple_GetItem(args, 0);
    if (msg == NULL) {
        PyErr_SetString(PyExc_TypeError, "log requires a message argument");
        return NULL;
    }
    Py_ssize_t nargs = PyTuple_GET_SIZE(args);
    PyObject *args_ = PyTuple_New(nargs - 1);
    if (args_ == nullptr)
        return nullptr;
    for (int i = 1; i < nargs; i++) {
        PyTuple_SET_ITEM(args_, i - 1, PyTuple_GET_ITEM(args, i));
        Py_INCREF(PyTuple_GET_ITEM(args, i)); // TODO: verify the old reference is discarded.
    }
    PyObject* exc_info = kwds != nullptr ? PyDict_GetItem(kwds, self->_const_exc_info) : nullptr;
    if (exc_info == nullptr){
        exc_info = Py_None;
        Py_INCREF(exc_info);
    } else {
        if (PyExceptionInstance_Check(exc_info)){
            PyObject * unpackedExcInfo = PyTuple_New(3);
            PyObject * excType = (PyObject*)Py_TYPE(exc_info);
            PyTuple_SET_ITEM(unpackedExcInfo, 0, excType);
            Py_INCREF(excType);
            PyTuple_SET_ITEM(unpackedExcInfo, 1, exc_info);
            Py_INCREF(exc_info);
            PyObject* traceback = PyObject_GetAttrString(exc_info, "__traceback__");
            PyTuple_SET_ITEM(unpackedExcInfo, 2, traceback);
            Py_INCREF(traceback);
            exc_info = unpackedExcInfo;
        } else if (!PyTuple_CheckExact(exc_info)){ // Probably Py_TRUE, fetch current exception as tuple
            PyObject * unpackedExcInfo = PyTuple_New(3);
            PyErr_GetExcInfo(&PyTuple_GET_ITEM(unpackedExcInfo, 0), &PyTuple_GET_ITEM(unpackedExcInfo, 1), &PyTuple_GET_ITEM(unpackedExcInfo, 2));
            exc_info = unpackedExcInfo;
        }
    }
    PyObject* extra = kwds != nullptr ? PyDict_GetItem(kwds, self->_const_extra) : nullptr;
    if (extra == nullptr){
        extra = Py_None;
        Py_INCREF(extra);
    }
    PyObject* stack_info = kwds != nullptr ? PyDict_GetItem(kwds, self->_const_stack_info) : nullptr;
    if (stack_info == nullptr){
        stack_info = Py_False;
        Py_INCREF(stack_info);
    }
    LogRecord *record = Logger_logMessageAsRecord(
        self, level, msg, args_, exc_info, extra, stack_info, 1);

    Py_DECREF(args_);
    Py_DECREF(exc_info);
    Py_DECREF(extra);
    Py_DECREF(stack_info);
    if (record == nullptr)
        return nullptr;

    if (Filterer_filter(&self->filterer, (PyObject*)record) != Py_True) {
        Py_DECREF(record);
        Py_RETURN_NONE;
    }
    
    int found = 0;
    Logger* cur = self;
    bool has_parent = true;
    while (has_parent){
        for (int i = 0; i < PyList_GET_SIZE(cur->handlers) ; i++){
            found ++;
            PyObject* handler = PyList_GET_ITEM(cur->handlers, i); // borrowed
            if (Handler_Check(handler)){
                if (record->levelno >= ((Handler*)handler)->level){
                    if (Handler_handle((Handler*)handler, (PyObject*)record) == nullptr){
                        Py_DECREF(record);
                        return nullptr;
                    }
                }
            } else {
                PyObject* handlerLevel = PyObject_GetAttr(handler, self->_const_level);
                if (handlerLevel == nullptr){
                    Py_DECREF(record);
                    PyErr_SetString(PyExc_TypeError, "Handler has no level attribute");
                    return nullptr;
                }
                
                if (record->levelno >= PyLong_AsLong(handlerLevel)){
                    if (PyObject_CallMethod_ONEARG(handler, self->_const_handle, (PyObject*)record) == nullptr){
                        Py_DECREF(handlerLevel);
                        Py_DECREF(record);
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
                Py_DECREF(record);
                PyErr_SetString(PyExc_TypeError, "Logger's parent is not an instance of picologging.Logger");
                return nullptr;
            }
            cur = (Logger*)cur->parent;
        }
    }
    if (found == 0){
        if (record->levelno >= ((Handler*)self->_fallback_handler)->level){
            if (Handler_handle((Handler*)self->_fallback_handler, (PyObject*)record) == nullptr){
                Py_DECREF(record);
                return nullptr;
            }
        }
    }
    Py_DECREF(record);
    Py_RETURN_NONE;
}

PyObject* Logger_debug(Logger *self, PyObject *args, PyObject *kwds) {
    if (self->disabled || !self->enabledForDebug) {
        Py_RETURN_NONE;
    }

    if (PyTuple_GET_SIZE(args) < 1){
        PyErr_SetString(PyExc_TypeError, "debug() requires 1 positional argument");
        return nullptr;
    }
    return Logger_logAndHandle(self, args, kwds, LOG_LEVEL_DEBUG);
}

PyObject* Logger_info(Logger *self, PyObject *args, PyObject *kwds){
    if (self->disabled || !self->enabledForInfo) {
        Py_RETURN_NONE;
    }

    if (PyTuple_GET_SIZE(args) < 1){
        PyErr_SetString(PyExc_TypeError, "info() requires 1 positional argument");
        return nullptr;
    }
    return Logger_logAndHandle(self, args, kwds, LOG_LEVEL_INFO);
}
PyObject* Logger_warning(Logger *self, PyObject *args, PyObject *kwds){
    if (self->disabled || !self->enabledForWarning) {
        Py_RETURN_NONE;
    }

    if (PyTuple_GET_SIZE(args) < 1){
        PyErr_SetString(PyExc_TypeError, "warning() requires 1 positional argument");
        return nullptr;
    }
    return Logger_logAndHandle(self, args, kwds, LOG_LEVEL_WARNING);
}

PyObject* Logger_fatal(Logger *self, PyObject *args, PyObject *kwds){
    return Logger_critical(self, args, kwds);
}

PyObject* Logger_error(Logger *self, PyObject *args, PyObject *kwds){
    if (self->disabled || !self->enabledForError) {
        Py_RETURN_NONE;
    }

    if (PyTuple_GET_SIZE(args) < 1){
        PyErr_SetString(PyExc_TypeError, "error() requires 1 positional argument");
        return nullptr;
    }
    return Logger_logAndHandle(self, args, kwds, LOG_LEVEL_ERROR);
}

PyObject* Logger_critical(Logger *self, PyObject *args, PyObject *kwds){
    if (self->disabled || !self->enabledForCritical) {
        Py_RETURN_NONE;
    }

    if (PyTuple_GET_SIZE(args) < 1){
        PyErr_SetString(PyExc_TypeError, "critical() requires 1 positional argument");
        return nullptr;
    }
    return Logger_logAndHandle(self, args, kwds, LOG_LEVEL_CRITICAL);
}

PyObject* Logger_exception(Logger *self, PyObject *args, PyObject *kwds){
    if (self->disabled || !self->enabledForError) {
        Py_RETURN_NONE;
    }
    if (kwds == nullptr){
        kwds = PyDict_New();
    }
    PyDict_SetItemString(kwds, "exc_info", Py_True);
    PyObject* result = Logger_logAndHandle(self, args, kwds, LOG_LEVEL_ERROR);
    Py_DECREF(kwds);
    return result;
}

PyObject* Logger_log(Logger *self, PyObject *args, PyObject *kwds){
    Py_ssize_t nargs = PyTuple_GET_SIZE(args);
    if (nargs < 2){
        PyErr_SetString(PyExc_TypeError, "log() requires at least 2 positional arguments");
        return nullptr;
    }
    if (!PyLong_Check(PyTuple_GET_ITEM(args, 0))){
        PyErr_SetString(PyExc_TypeError, "log() requires a level argument");
        return nullptr;
    }
    unsigned short level = PyLong_AsUnsignedLongMask(PyTuple_GET_ITEM(args, 0));

    if (self->disabled || (self->level > level)) {
        Py_RETURN_NONE;
    }

    PyObject *args_ = PyTuple_New(nargs - 1);
    if (args_ == nullptr)
        return nullptr;
    for (int i = 1; i < nargs; i++) {
        PyTuple_SET_ITEM(args_, i - 1, PyTuple_GET_ITEM(args, i));
        Py_INCREF(PyTuple_GET_ITEM(args, i)); // TODO: verify the old reference is discarded.
    }

    PyObject* result = Logger_logAndHandle(self, args_, kwds, level);
    Py_DECREF(args_);
    return result;
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
        PyObject* remove = PyUnicode_FromString("remove");
        PyObject* result = PyObject_CallMethod_ONEARG(self->handlers, remove, handler);
        Py_DECREF(remove);
        return result;
    }
    Py_RETURN_NONE;
}

static PyObject *
Logger_get_parent(Logger *self, void *closure)
{
    if (self->parent == nullptr) {
        Py_RETURN_NONE;
    }
    Py_INCREF(self->parent);
    return self->parent;
}

static int
Logger_set_parent(Logger *self, PyObject *value, void *Py_UNUSED(ignored))
{
    if (value == nullptr) {
        PyErr_SetString(PyExc_TypeError, "Cannot delete parent");
        return -1;
    }
    if (!Logger_Check(value)) {
        PyErr_Format(PyExc_TypeError, "parent must be a Logger, not %s", Py_TYPE(value)->tp_name);
        return -1;
    }
    Py_XINCREF(value);
    Py_XDECREF(self->parent);
    self->parent = value;
    // Rescan parent levels.
    self->enabledForDebug = false;
    self->enabledForInfo = false;
    self->enabledForWarning = false;
    self->enabledForError = false;
    self->enabledForCritical = false;
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
    return 0;
}

static PyMethodDef Logger_methods[] = {
    {"setLevel", (PyCFunction)Logger_setLevel, METH_O, "Set the level of the logger."},
    {"getEffectiveLevel", (PyCFunction)Logger_getEffectiveLevel, METH_NOARGS, "Get the effective level of the logger."},
    {"addHandler", (PyCFunction)Logger_addHandler, METH_O, "Add a handler to the logger."},
    {"removeHandler", (PyCFunction)Logger_removeHandler, METH_O, "Remove a handler from the logger."},
    // Logging methods
    {"debug", (PyCFunction)Logger_debug, METH_VARARGS | METH_KEYWORDS, "Log a message at level DEBUG."},
    {"info", (PyCFunction)Logger_info, METH_VARARGS | METH_KEYWORDS, "Log a message at level INFO."},
    {"warning", (PyCFunction)Logger_warning, METH_VARARGS | METH_KEYWORDS, "Log a message at level WARNING."},
    {"error", (PyCFunction)Logger_error, METH_VARARGS | METH_KEYWORDS, "Log a message at level ERROR."},
    {"critical", (PyCFunction)Logger_critical, METH_VARARGS | METH_KEYWORDS, "Log a message at level CRITICAL."},
    {"exception", (PyCFunction)Logger_exception, METH_VARARGS | METH_KEYWORDS, "Log a message at level ERROR."},
    {"fatal", (PyCFunction)Logger_fatal, METH_VARARGS | METH_KEYWORDS, "Log a message at level FATAL."},
    {"log", (PyCFunction)Logger_log, METH_VARARGS | METH_KEYWORDS, "Log a message at the specified level."},
    {NULL}
};

static PyMemberDef Logger_members[] = {
    {"name", T_OBJECT_EX, offsetof(Logger, name), 0, "Logger name"},
    {"level", T_USHORT, offsetof(Logger, level), 0, "Logger level"},
    {"propagate", T_BOOL, offsetof(Logger, propagate), 0, "Logger propagate"},
    {"handlers", T_OBJECT_EX, offsetof(Logger, handlers), 0, "Logger handlers"},
    {"disabled", T_BOOL, offsetof(Logger, disabled), 0, "Logger disabled"},
    {"manager", T_OBJECT_EX, offsetof(Logger, manager), 0, "Logger manager"},
    {NULL}
};

static PyGetSetDef Logger_getsets[] = {
    {"parent",
     (getter)Logger_get_parent,
     (setter)Logger_set_parent,
     "Logger parent"},
    {NULL, NULL, NULL, NULL }  /* sentinel */
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
    Logger_methods,                             /* tp_methods */
    Logger_members,                             /* tp_members */
    Logger_getsets,                             /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    (initproc)Logger_init,                      /* tp_init */
    0,                                          /* tp_alloc */
    Logger_new,                                 /* tp_new */
    PyObject_Del,                               /* tp_free */
};

