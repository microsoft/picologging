#include "handler.hxx"
#include "picologging.hxx"
#include "formatter.hxx"

int Handler_init(Handler *self, PyObject *args, PyObject *kwds){
    if (FiltererType.tp_init((PyObject *) self, args, kwds) < 0)
        return -1;
    PyObject *name = Py_None;
    unsigned short level = LOG_LEVEL_NOTSET;
    PyObject *formatter = NULL;
    PyObject *filters = NULL;
    static char *kwlist[] = {"name", "level", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OHOO", kwlist, &name, &level)){
        return -1;
    }
    self->name = name;
    Py_INCREF(name);

    self->level = level;
    self->formatter = Py_None;
    Py_INCREF(self->formatter);
    self->filters = PyList_New(0);
    Py_INCREF(self->filters);

    return 0;
}

PyObject* Handler_dealloc(Handler *self) {
    Py_XDECREF(self->name);
    Py_XDECREF(self->formatter);
    Py_XDECREF(self->filters);
    ((PyObject*)self)->ob_type->tp_free((PyObject*)self);
    return nullptr;
}

PyObject* Handler_emit(Handler *self, PyObject *record){
    Py_RETURN_NONE;
}

PyObject* Handler_handle(Handler *self, PyObject *record) {
    if (Filterer_filter(&self->filterer, (PyObject*)record) != Py_True)
        Py_RETURN_NONE;
    const std::lock_guard<std::mutex> lock(self->lock);
    return Handler_emit(self, record);
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

PyObject* Handler_setFormatter(Handler *self, PyObject *formatter) {
    Py_XDECREF(self->formatter);
    self->formatter = formatter;
    Py_INCREF(self->formatter);
    Py_RETURN_NONE;
}

static PyMethodDef Handler_methods[] = {
    {"setLevel", (PyCFunction)Handler_setLevel, METH_O, "Set the level of the handler."},
    {"setFormatter", (PyCFunction)Handler_setFormatter, METH_O, "Set the formatter of the handler."},
    {"handle", (PyCFunction)Handler_handle, METH_O, "Handle a record."},
    {"emit", (PyCFunction)Handler_emit, METH_O, "Emit a record."},
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
    PyType_GenericNew,                          /* tp_new */
    PyObject_Del,                               /* tp_free */
};

