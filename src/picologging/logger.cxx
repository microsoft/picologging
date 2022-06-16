#include "logger.hxx"
#include "compat.hxx"

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
    self->filters = PyList_New(0);
    Py_INCREF(self->filters);
    return 0;
}

PyObject* Logger_dealloc(Logger *self) {
    Py_XDECREF(self->name);
    Py_XDECREF(self->parent);
    Py_XDECREF(self->handlers);
    Py_XDECREF(self->_cache);
    Py_XDECREF(self->filters);
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
    PyObject* logger = (PyObject*)self;
    while (logger != Py_None) {
        // TODO : We could support logging.Logger here through duck-typing..
        // It depends on whether this is requested by the users or not.
        if (!Logger_CheckExact(logger)) {
            PyErr_SetString(PyExc_TypeError, "Parent logger is not a picologging.Logger");
            return NULL;
        }
        if (((Logger*)logger)->level > 0){
            return PyLong_FromUnsignedLong(((Logger*)logger)->level);
        }
        logger = ((Logger*)logger)->parent;
        continue;
    }
    return PyLong_FromLong(0);
}

PyObject* Logger_addFilter(Logger* self, PyObject *filter) {
    // Equivalent to `if not (filter in self.filters):`
    if (PySequence_Contains(self->filters, filter) == 0){
        PyList_Append(self->filters, filter);
    }
    Py_RETURN_NONE;
}

PyObject* Logger_removeFilter(Logger* self, PyObject *filter) {
    if (PySequence_Contains(self->filters, filter) == 1){
        return PyObject_CallMethod_ONEARG(self->filters, PyUnicode_FromString("remove"), filter);
    }
    Py_RETURN_NONE;
}

PyObject* Logger_filter(Logger* self, PyObject *record) {
    bool ret = true;
    for (int i = 0; i < PyList_GET_SIZE(self->filters); i++) {
        PyObject *result = Py_None;
        PyObject *filter = PyList_GET_ITEM(self->filters, i); // borrowed ref
        if (PyObject_HasAttrString(filter, "filter")) {
            result = PyObject_CallMethod_ONEARG(filter, PyUnicode_FromString("filter"), record);
            if (result == NULL) {
                return NULL;
            }
        } else {
            result = PyObject_CallFunctionObjArgs(filter, record, NULL);
        }
        if (result == Py_False || result == Py_None) {
            ret = false;
            break;
        }
    }

    if (ret)
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

static PyMethodDef Logger_methods[] = {
    {"setLevel", (PyCFunction)Logger_setLevel, METH_O, "Set the level of the logger."},
    {"getEffectiveLevel", (PyCFunction)Logger_getEffectiveLevel, METH_NOARGS, "Get the effective level of the logger."},
    {"addFilter", (PyCFunction)Logger_addFilter, METH_O, "Add a filter to the logger."},
    {"removeFilter", (PyCFunction)Logger_removeFilter, METH_O, "Remove a filter from the logger."},
    {"filter", (PyCFunction)Logger_filter, METH_O, "Filter a record."},
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
    {"filters", T_OBJECT_EX, offsetof(Logger, filters), 0, "Logger filters"},
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

