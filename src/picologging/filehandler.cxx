#include <mutex>

#include "filehandler.hxx"
#include "handler.hxx"
#include "compat.hxx"

PyObject* FileHandler_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    FileHandler* self = (FileHandler*)HandlerType.tp_new(type, args, kwds);
    if (self != NULL){
        self->stream = Py_None;
        self->terminator = PyUnicode_FromString("\n");
        self->_const_io_mod = PyImport_ImportModule("io");
    }
    return (PyObject*)self;
}

int FileHandler_init(FileHandler *self, PyObject *args, PyObject *kwds){
    if (HandlerType.tp_init((PyObject *) self, args, kwds) < 0)
        return -1;

    PyObject *filename = NULL;
    PyObject *mode = NULL;
    PyObject *encoding = NULL;
    bool delay = false;
    PyObject *errors = NULL;
    static const char *kwlist[] = {"filename", "mode", "encoding", "delay", "errors", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|OOpO", const_cast<char**>(kwlist),
        &filename, &mode, &encoding, &delay, &errors))
        return -1;

    // TODO: Use delay
    filename = PyOS_FSPath(filename);

    self->stream = PyObject_CallMethod(self->_const_io_mod, "open", "ss", PyUnicode_AsUTF8(filename), "ab");
    Py_INCREF(self->stream);

    // As a result of PyOS_FSPath
    Py_DECREF(filename);
    return 0;
}

PyObject* FileHandler_dealloc(FileHandler *self) {
    Py_DECREF(self->stream);
    Py_DECREF(self->terminator);
    Py_DECREF(self->_const_io_mod);
    ((PyObject*)self)->ob_type->tp_free((PyObject*)self);
    return nullptr;
}

PyObject* FileHandler_emit(FileHandler* self, PyObject* const* args, Py_ssize_t nargs){
    if (nargs < 1){
        PyErr_SetString(PyExc_ValueError, "emit() takes at least 1 argument");
        return nullptr;
    }
    PyObject* msg = Handler_format(&self->handler, args[0]);
    if (msg == nullptr)
        return nullptr;
    if (!PyUnicode_CheckExact(msg)){
        PyErr_SetString(PyExc_TypeError, "Result of self.handler.format() must be a string");
        goto error;
    }
    PyUnicode_Append(&msg, self->terminator);
    if (PyObject_CallMethod(self->stream, "write", "y", PyUnicode_AsUTF8(msg)) == nullptr) {
        if (!PyErr_Occurred())
            PyErr_SetString(PyExc_RuntimeError, "Cannot write to file");
        goto error;
    }
    FileHandler_flush(self);
    Py_DECREF(msg);
    Py_RETURN_NONE;
error:
    // TODO: #4 handle error path (see handleError(record))
    Py_XDECREF(msg);
    return nullptr;
}

PyObject* FileHandler_flush(FileHandler* self){
    Handler_acquire(&self->handler);
    PyObject_CallMethod(self->stream, "flush", NULL);
    Handler_release(&self->handler);
    Py_RETURN_NONE;
}

PyObject* FileHandler_close(FileHandler* self) {
    Handler_acquire(&self->handler);
    PyObject_CallMethod(self->stream, "close", NULL);
    Handler_release(&self->handler);
    Py_RETURN_NONE;
}

static PyMethodDef FileHandler_methods[] = {
     {"emit", (PyCFunction)FileHandler_emit, METH_FASTCALL, "Emit a record."},
     {"flush", (PyCFunction)FileHandler_flush, METH_NOARGS, "Flush the stream."},
     {"close", (PyCFunction)FileHandler_close, METH_NOARGS, "Close the stream."},
     {NULL}
};

static PyMemberDef FileHandler_members[] = {
    // {"stream", T_OBJECT_EX, offsetof(FileHandler, stream), 0, "Stream"},
    {NULL}
};

PyTypeObject FileHandlerType = {
    PyObject_HEAD_INIT(NULL)
    "picologging.FileHandler",                    /* tp_name */
    sizeof(FileHandler),                          /* tp_basicsize */
    0,                                            /* tp_itemsize */
    (destructor)FileHandler_dealloc,              /* tp_dealloc */
    0,                                            /* tp_vectorcall_offset */
    0,                                            /* tp_getattr */
    0,                                            /* tp_setattr */
    0,                                            /* tp_as_async */
    0,                                            /* tp_repr */
    0,                                            /* tp_as_number */
    0,                                            /* tp_as_sequence */
    0,                                            /* tp_as_mapping */
    0,                                            /* tp_hash */
    0,                                            /* tp_call */
    0,                                            /* tp_str */
    PyObject_GenericGetAttr,                      /* tp_getattro */
    PyObject_GenericSetAttr,                      /* tp_setattro */
    0,                                            /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE ,    /* tp_flags */
    PyDoc_STR("FileHandler interface."),          /* tp_doc */
    0,                                            /* tp_traverse */
    0,                                            /* tp_clear */
    0,                                            /* tp_richcompare */
    0,                                            /* tp_weaklistoffset */
    0,                                            /* tp_iter */
    0,                                            /* tp_iternext */
    FileHandler_methods,                          /* tp_methods */
    FileHandler_members,                          /* tp_members */
    0,                                            /* tp_getset */
    0,                                            /* tp_base */
    0,                                            /* tp_dict */
    0,                                            /* tp_descr_get */
    0,                                            /* tp_descr_set */
    0,                                            /* tp_dictoffset */
    (initproc)FileHandler_init,                   /* tp_init */
    0,                                            /* tp_alloc */
    FileHandler_new,                              /* tp_new */
    PyObject_Del,                                 /* tp_free */
};
