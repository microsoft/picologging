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
        self->_const_prepare = PyUnicode_FromString("prepare");
        self->_const_enqueue = PyUnicode_FromString("enqueue");
        self->queue = Py_None;
    }
    return (PyObject*)self;
}

int QueueHandler_init(QueueHandler *self, PyObject *args, PyObject *kwds){
    if (HandlerType.tp_init((PyObject *) self, args, kwds) < 0)
        return -1;
    PyObject *queue = NULL;
    if (!PyArg_ParseTuple(args, "O", &queue)){
        return -1;
    }
    self->queue = Py_NewRef(queue);
    return 0;
}

PyObject* QueueHandler_dealloc(QueueHandler *self) {
    Py_CLEAR(self->queue);
    Py_CLEAR(self->_const_put_nowait);
    Py_CLEAR(self->_const_prepare);
    Py_CLEAR(self->_const_enqueue);
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
    /*
    """ (From the Python implementation -- )
        Prepare a record for queuing. The object returned by this method is
        enqueued.

        The base implementation formats the record to merge the message and
        arguments, and removes unpickleable items from the record in-place.
        Specifically, it overwrites the record's `msg` and
        `message` attributes with the merged message (obtained by
        calling the handler's `format` method), and sets the `args`,
        `exc_info` and `exc_text` attributes to None.

        You might want to override this method if you want to convert
        the record to a dict or JSON string, or send a modified copy
        of the record while leaving the original intact.
        """
        # The format operation gets traceback text into record.exc_text
        # (if there's exception data), and also returns the formatted
        # message. We can then use this to replace the original
        # msg + args, as these might be unpickleable. We also zap the
        # exc_info, exc_text and stack_info attributes, as they are no longer
        # needed and, if not None, will typically not be pickleable.
        */
    if (LogRecord_CheckExact(record)){
        PyObject* msg = Handler_format(&self->handler, record);
        if (msg == nullptr){
            return nullptr;
        }
        // Create new log record..
        LogRecord* newRecord = (LogRecord*) (&LogRecordType)->tp_alloc(&LogRecordType, 0);
        if (newRecord == NULL)
        {
            PyErr_NoMemory();
            return nullptr;
        }
        return (PyObject*)LogRecord_create(
            newRecord,
            ((LogRecord*)record)->name,
            msg,
            Py_None,
            ((LogRecord*)record)->levelno,
            ((LogRecord*)record)->filename,
            ((LogRecord*)record)->lineno,
            Py_None,
            ((LogRecord*)record)->funcName,
            ((LogRecord*)record)->stackInfo
        );
    }
    else{
        PyErr_SetString(PyExc_ValueError, "prepare() takes a LogRecord as argument");
        return nullptr;
    }
}

PyObject* QueueHandler_emit(QueueHandler* self, PyObject* record){
    // Pass record to .prepare()
    PyObject* prepareResult = nullptr, *enqueueResult = nullptr;
    if (QueueHandler_CheckExact((PyObject*)self)){
        prepareResult = QueueHandler_prepare(self, record);
    } else {
        prepareResult = PyObject_CallMethod_ONEARG((PyObject*)self, self->_const_prepare, record);
    }
    if (prepareResult == nullptr) {
        Handler_handleError(&self->handler, record);
        Py_RETURN_NONE;
    }
    // TODO : Decref the old record?
    // Enqueue result of .prepare()
    if (QueueHandler_CheckExact((PyObject*)self)){
        enqueueResult = QueueHandler_enqueue(self, prepareResult);
    } else {
        enqueueResult = PyObject_CallMethod_ONEARG((PyObject*)self, self->_const_enqueue, prepareResult);
    }
    if (enqueueResult == nullptr) {
        Handler_handleError(&self->handler, record);
        Py_RETURN_NONE;
    }
    Py_DECREF(enqueueResult);
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

