#include "logger.hxx"

int Logger_init(Logger *self, PyObject *args, PyObject *kwds)
{
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
    self->_cache = PyDict_New();
    Py_INCREF(self->_cache);
    return 0;
}

PyObject* Logger_dealloc(Logger *self) {
    Py_XDECREF(self->name);
    Py_XDECREF(self->parent);
    Py_XDECREF(self->handlers);
    Py_XDECREF(self->_cache);
    Py_TYPE(self)->tp_free((PyObject*)self);
    return NULL;
}

PyObject* Logger_repr(Logger *self) {
    return PyUnicode_FromFormat("<Logger '%U' (%d)>", self->name, self->level);
}

PyObject* Logger_setLevel(Logger *self, PyObject *args) {
    unsigned short level = 0;
    if (!PyArg_ParseTuple(args, "H", &level))
        return NULL;
    self->level = level;
    Py_RETURN_NONE;
}

static PyMethodDef Logger_methods[] = {
    {"setLevel", (PyCFunction)Logger_setLevel, METH_VARARGS, "Set the level of the logger."},
    {NULL}
};

static PyMemberDef Logger_members[] = {
    {"name", T_OBJECT_EX, offsetof(Logger, name), 0, "Logger name"},
    {"level", T_USHORT, offsetof(Logger, level), 0, "Logger level"},
    {"parent", T_OBJECT_EX, offsetof(Logger, parent), 0, "Logger parent"},
    {"propagate", T_BOOL, offsetof(Logger, propagate), 0, "Logger propagate"},
    {"handlers", T_OBJECT_EX, offsetof(Logger, handlers), 0, "Logger handlers"},
    {"disabled", T_BOOL, offsetof(Logger, disabled), 0, "Logger disabled"},
    {"_cache", T_OBJECT_EX, offsetof(Logger, _cache), 0, "Logger _cache"},
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

