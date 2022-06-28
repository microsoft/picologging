#include <mutex>

#include "handler.hxx"
#include "picologging.hxx"
#include "formatter.hxx"
#include "streamhandler.hxx"

PyObject* Handler_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    Handler* self = (Handler*)FiltererType.tp_new(type, args, kwds);
    if (self != NULL)
    {
        self->lock = new std::recursive_mutex();
        self->_const_emit = PyUnicode_FromString("emit");
        self->_const_format = PyUnicode_FromString("format");
        self->name = Py_None;
        self->formatter = Py_None;
        Py_INCREF(self->formatter);
    }
    return (PyObject*)self;
}

int Handler_init(Handler *self, PyObject *args, PyObject *kwds){
    if (FiltererType.tp_init((PyObject *) self, args, kwds) < 0)
        return -1;
    PyObject *name = Py_None;
    unsigned short level = LOG_LEVEL_NOTSET;
    static const char *kwlist[] = {"name", "level", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OH", const_cast<char**>(kwlist), &name, &level)){
        return -1;
    }
    self->name = name;
    Py_INCREF(name);
    self->level = level;
    return 0;
}

PyObject* Handler_dealloc(Handler *self) {
    Py_XDECREF(self->name);
    Py_XDECREF(self->formatter);
    Py_XDECREF(self->_const_emit);
    Py_XDECREF(self->_const_format);
    delete self->lock;
    ((PyObject*)self)->ob_type->tp_free((PyObject*)self);
    return nullptr;
}

PyObject* Handler_emit(Handler *self, PyObject *record){
    Py_RETURN_NOTIMPLEMENTED;
}

PyObject* Handler_handle(Handler *self, PyObject *record) {
    if (Filterer_filter(&self->filterer, (PyObject*)record) != Py_True)
        Py_RETURN_NONE;

    try {
        self->lock->lock();
    } catch (const std::exception& e) {
        PyErr_Format(PyExc_RuntimeError, "Cannot acquire thread lock, %s", e.what());
        return nullptr;
    }
    PyObject* result = nullptr;
    if (StreamHandler_CheckExact(((PyObject*)self))){
        PyObject* args[1] = {record};
        result = StreamHandler_emit((StreamHandler*)self, args, 1);
    } else {
        result = PyObject_CallMethod_ONEARG((PyObject*)self, self->_const_emit, record);
    }
    
    self->lock->unlock();
    return result;
}

PyObject* Handler_setLevel(Handler *self, PyObject *level){
    if (PyLong_Check(level)){
        self->level = PyLong_AsUnsignedLong(level);
        Py_RETURN_NONE;
    } else {
        PyErr_SetString(PyExc_TypeError, "level must be an integer");
        return nullptr;
    }
}

PyObject* Handler_format(Handler *self, PyObject *record){
    if (self->formatter == Py_None){
        // Lazily initialize default formatter..
        Py_DECREF(self->formatter);
        self->formatter = PyObject_CallFunctionObjArgs((PyObject*)&FormatterType, NULL);
        if (self->formatter == nullptr){
            // Reset to none if we failed to initialize
            self->formatter = Py_None;
            Py_INCREF(self->formatter);
            return nullptr;
        }
    }

    if (Formatter_CheckExact(self->formatter)) {
        return Formatter_format((Formatter*) self->formatter, record);
    } else {
        return PyObject_CallMethod_ONEARG(self->formatter, self->_const_format, record);
    }
}

PyObject* Handler_setFormatter(Handler *self, PyObject *formatter) {
    Py_XDECREF(self->formatter);
    self->formatter = formatter;
    Py_INCREF(self->formatter);
    Py_RETURN_NONE;
}

PyObject* Handler_acquire(Handler *self){
    self->lock->lock();
    Py_RETURN_NONE;
}

PyObject* Handler_release(Handler *self){
    self->lock->unlock();
    Py_RETURN_NONE;
}

PyObject* Handler_flush(Handler *self){
    //Abstract method. does nothing.
    Py_RETURN_NONE;
}

PyObject* Handler_close(Handler *self){
    // TODO: Decide if we want a global dictionary of handlers.
    Py_RETURN_NONE;
}

PyObject* Handler_handleError(Handler *self, PyObject *record){
    // TODO: Develop this behaviour further.
    PyErr_Print();
    Py_RETURN_NONE;
}

PyObject* Handler_getName(Handler *self){
    Py_INCREF(self->name);
    return self->name;
}

PyObject* Handler_setName(Handler *self, PyObject *name){
    Py_XDECREF(self->name);
    self->name = name;
    Py_INCREF(self->name);
    Py_RETURN_NONE;
}

PyObject* Handler_createLock(Handler *self){
    // Lock is instantiated by constructor, just have this method for compatibility with logging.Handler
    Py_RETURN_NONE;
}

static PyMethodDef Handler_methods[] = {
    {"setLevel", (PyCFunction)Handler_setLevel, METH_O, "Set the level of the handler."},
    {"setFormatter", (PyCFunction)Handler_setFormatter, METH_O, "Set the formatter of the handler."},
    {"handle", (PyCFunction)Handler_handle, METH_O, "Handle a record."},
    {"emit", (PyCFunction)Handler_emit, METH_O, "Emit a record."},
    {"format", (PyCFunction)Handler_format, METH_O, "Format a record."},
    {"acquire", (PyCFunction)Handler_acquire, METH_NOARGS, "Acquire the lock."},
    {"release", (PyCFunction)Handler_release, METH_NOARGS, "Release the lock."},
    {"flush", (PyCFunction)Handler_flush, METH_NOARGS, "Ensure all logging output has been flushed."},
    {"close", (PyCFunction)Handler_close, METH_NOARGS, "Tidy up any resources used by the handler."},
    {"handleError", (PyCFunction)Handler_handleError, METH_O, "Handle an error during an emit()."},
    {"get_name", (PyCFunction)Handler_getName, METH_NOARGS, "Get the name of the handler."},
    {"set_name", (PyCFunction)Handler_setName, METH_O, "Set the name of the handler."},
    {"createLock", (PyCFunction)Handler_createLock, METH_NOARGS, "Create a new lock instance."},
    {NULL}
};

static PyMemberDef Handler_members[] = {
    {"name", T_OBJECT_EX, offsetof(Handler, name), 0, "Handler name"},
    {"level", T_USHORT, offsetof(Handler, level), 0, "Handler level"},
    {"formatter", T_OBJECT_EX, offsetof(Handler, formatter), 0, "Handler formatter"},
    {NULL}
};

PyTypeObject HandlerType = {
    PyObject_HEAD_INIT(NULL)
    "picologging.Handler",                    /* tp_name */
    sizeof(Handler),                          /* tp_basicsize */
    0,                                          /* tp_itemsize */
    (destructor)Handler_dealloc,                /* tp_dealloc */
    0,                                          /* tp_vectorcall_offset */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_as_async */
    0,                      /* tp_repr */
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
    PyDoc_STR("Handler interface."),    /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    Handler_methods,                          /* tp_methods */
    Handler_members,                          /* tp_members */
    0,                                          /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    (initproc)Handler_init,                   /* tp_init */
    0,                                          /* tp_alloc */
    Handler_new,                          /* tp_new */
    PyObject_Del,                               /* tp_free */
};

