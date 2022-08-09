#include "filterer.hxx"
#include "compat.hxx"

PyObject* Filterer_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    Filterer* self = (Filterer*)type->tp_alloc(type, 0);
    if (self != NULL)
    {
        self->filters = PyList_New(0);
        if (self->filters == NULL)
            return nullptr;
        Py_INCREF(self->filters);
        self->_const_filter = PyUnicode_FromString("filter");
        self->_const_remove = PyUnicode_FromString("remove");
    }
    return (PyObject*)self;
}

int Filterer_init(Filterer *self, PyObject *args, PyObject *kwds)
{
    return 0;
}

PyObject* Filterer_addFilter(Filterer* self, PyObject *filter) {
    // Equivalent to `if not (filter in self.filters):`
    if (PySequence_Contains(self->filters, filter) == 0){
        PyList_Append(self->filters, filter);
    }
    Py_RETURN_NONE;
}

PyObject* Filterer_removeFilter(Filterer* self, PyObject *filter) {
    if (PySequence_Contains(self->filters, filter) == 1){
        return PyObject_CallMethod_ONEARG(self->filters, self->_const_remove, filter);
    }
    Py_RETURN_NONE;
}

PyObject* Filterer_filter(Filterer* self, PyObject *record) {
    bool ret = true;
    for (int i = 0; i < PyList_GET_SIZE(self->filters); i++) {
        PyObject *result = Py_None;
        PyObject *filter = PyList_GET_ITEM(self->filters, i); // borrowed ref
        if (PyObject_HasAttr(filter, self->_const_filter)) {
            result = PyObject_CallMethod_ONEARG(filter, self->_const_filter, record);
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

PyObject* Filterer_dealloc(Filterer *self) {
    Py_XDECREF(self->filters);
    Py_XDECREF(self->_const_filter);
    Py_XDECREF(self->_const_remove);
    Py_TYPE(self)->tp_free((PyObject*)self);
    return NULL;
}

static PyMethodDef Filterer_methods[] = {
    {"addFilter", (PyCFunction)Filterer_addFilter, METH_O, "Add a filter to the logger."},
    {"removeFilter", (PyCFunction)Filterer_removeFilter, METH_O, "Remove a filter from the logger."},
    {"filter", (PyCFunction)Filterer_filter, METH_O, "Filter a record."},
    NULL
};

static PyMemberDef Filterer_members[] = {
    {"filters", T_OBJECT_EX, offsetof(Filterer, filters), 0, "Filters"},
    {NULL}
};

PyTypeObject FiltererType = {
    PyObject_HEAD_INIT(NULL)
    "picologging.Filterer",                    /* tp_name */
    sizeof(Filterer),                          /* tp_basicsize */
    0,                                          /* tp_itemsize */
    (destructor)Filterer_dealloc,                /* tp_dealloc */
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
    PyDoc_STR("Filterer interface."),    /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    Filterer_methods,                          /* tp_methods */
    Filterer_members,                          /* tp_members */
    0,                                          /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    (initproc)Filterer_init,                   /* tp_init */
    0,                                          /* tp_alloc */
    Filterer_new,                          /* tp_new */
    PyObject_Del,                               /* tp_free */
};
