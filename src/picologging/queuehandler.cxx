#include "queuehandler.hxx"
#include "handler.hxx"
#include "compat.hxx"
#include "picologging.hxx"
#include "logrecord.hxx"
#include "handler.hxx"

PyObject* QueueHandler_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    QueueHandler* self = (QueueHandler*)HandlerType.tp_new(type, args, kwds);
    if (self != NULL)
    {
        self->_const_put_nowait = PyUnicode_FromString("put_nowait");
        self->queue = Py_None;
    }
    return (PyObject*)self;
}

int QueueHandler_init(QueueHandler *self, PyObject *args, PyObject *kwds){
    if (HandlerType.tp_init((PyObject *) self, args, kwds) < 0)
        return -1;
    PyObject *queue = NULL;
    static const char *kwlist[] = {"queue", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|O", const_cast<char**>(kwlist), &queue)){
        return -1;
    }
    self->queue = Py_NewRef(queue);
    return 0;
}

PyObject* QueueHandler_dealloc(QueueHandler *self) {
    Py_CLEAR(self->queue);
    Py_CLEAR(self->_const_put_nowait);
    HandlerType.tp_dealloc((PyObject *)self);
    return nullptr;
}

PyObject* QueueHandler_enqueue(QueueHandler* self, PyObject* record){
    PyObject* result = PyObject_CallMethod_ONEARG(self->queue, self->_const_put_nowait, record);
    if (result == nullptr)
        return nullptr;
    Py_XDECREF(result);
    Py_RETURN_NONE;
}

PyObject* QueueHandler_prepare(QueueHandler* self, PyObject* record){
    if (LogRecord_CheckExact(record)){
        PyObject* msg = Handler_format(&self->handler, record);
        if (msg == nullptr){
            return nullptr;
        }
        // Create new log record..

        // Return record
        return (PyObject*) record;
    }
    else{
        PyErr_SetString(PyExc_ValueError, "prepare() takes a LogRecord as argument");
        return nullptr;
    }
    Py_RETURN_NONE;
}

PyObject* QueueHandler_emit(QueueHandler* self, PyObject* record){
    // Pass record to .prepare()
    PyObject* prepareResult = nullptr, *enqueueResult = nullptr;
    prepareResult = QueueHandler_prepare(self, record);
    if (prepareResult == nullptr) {
        Handler_handleError(&self->handler, record);
        Py_RETURN_NONE;
    }
    // Enqueue result of .prepare()
    enqueueResult = QueueHandler_enqueue(self, prepareResult);
    if (enqueueResult == nullptr) {
        Handler_handleError(&self->handler, record);
        Py_RETURN_NONE;
    }
    Py_RETURN_NONE;
}

PyObject* QueueHandler_repr(QueueHandler *self)
{
    std::string level = _getLevelName(self->handler.level);
    PyObject* repr = PyUnicode_FromFormat("<%s (%s)>",
        _PyType_Name(Py_TYPE(self)),
        level.c_str());
    return repr;
}

static PyMethodDef QueueHandler_methods[] = {
     {"enqueue", (PyCFunction)QueueHandler_enqueue, METH_O, "Enqueue a record."},
     {"prepare", (PyCFunction)QueueHandler_prepare, METH_O, "Prepare a record."},
     {"emit", (PyCFunction)QueueHandler_emit, METH_O, "Emit a record."},
     {NULL}
};

static PyMemberDef QueueHandler_members[] = {
    {"queue", T_OBJECT_EX, offsetof(QueueHandler, queue), 0, "Queue"},
    {NULL}
};

PyTypeObject QueueHandlerType = {
    PyObject_HEAD_INIT(NULL)
    "picologging.QueueHandler",                    /* tp_name */
    sizeof(QueueHandler),                          /* tp_basicsize */
    0,                                          /* tp_itemsize */
    (destructor)QueueHandler_dealloc,                /* tp_dealloc */
    0,                                          /* tp_vectorcall_offset */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_as_async */
   (reprfunc)QueueHandler_repr,                /* tp_repr */
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
    PyDoc_STR("QueueHandler interface."),    /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    QueueHandler_methods,                          /* tp_methods */
    QueueHandler_members,                          /* tp_members */
    0,                                          /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    (initproc)QueueHandler_init,                   /* tp_init */
    0,                                          /* tp_alloc */
    QueueHandler_new,                          /* tp_new */
    PyObject_Del,                               /* tp_free */
};

