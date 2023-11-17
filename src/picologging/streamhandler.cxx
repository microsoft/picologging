#include <mutex>

#include "streamhandler.hxx"
#include "handler.hxx"
#include "compat.hxx"
#include "picologging.hxx"

PyObject* StreamHandler_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    StreamHandler* self = (StreamHandler*)HandlerType.tp_new(type, args, kwds);
    if (self != NULL)
    {
        self->terminator = PyUnicode_FromString("\n");
        self->_const_write = PyUnicode_FromString("write");
        self->_const_flush = PyUnicode_FromString("flush");
        self->stream = Py_None;
        self->stream_has_flush = false;
    }
    return (PyObject*)self;
}

int StreamHandler_init(StreamHandler *self, PyObject *args, PyObject *kwds){
    if (HandlerType.tp_init((PyObject *) self, args, kwds) < 0)
        return -1;
    PyObject *stream = NULL;
    static const char *kwlist[] = {"stream", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|O", const_cast<char**>(kwlist), &stream)){
        return -1;
    }
    if (stream == NULL || stream == Py_None){
        stream = PySys_GetObject("stderr");
    }
    self->stream = Py_NewRef(stream);
    self->stream_has_flush = (PyObject_HasAttr(self->stream, self->_const_flush) == 1);
    return 0;
}

PyObject* StreamHandler_dealloc(StreamHandler *self) {
    Py_CLEAR(self->stream);
    Py_CLEAR(self->terminator);
    Py_CLEAR(self->_const_write);
    Py_CLEAR(self->_const_flush);
    HandlerType.tp_dealloc((PyObject *)self);
    return nullptr;
}

PyObject* flush (StreamHandler* self){
    if (!self->stream_has_flush)
        Py_RETURN_NONE;
    Handler_acquire(&self->handler);
    PyObject* result = PyObject_CallMethod_NOARGS(self->stream, self->_const_flush);
    Py_XDECREF(result);
    Handler_release(&self->handler);
    Py_RETURN_NONE;
}

PyObject* StreamHandler_emit(StreamHandler* self, PyObject* const* args, Py_ssize_t nargs){
    PyObject* writeResult = nullptr;
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
    if (msg == nullptr) { // PyUnicode_Append sets *pleft to null on error. Error is extremely unlikely
        goto error;
    }
    writeResult = PyObject_CallMethod_ONEARG(self->stream, self->_const_write, msg);
    if (writeResult == nullptr){
        if (!PyErr_Occurred())
            PyErr_SetString(PyExc_RuntimeError, "Cannot write to stream");
        goto error;
    }
    flush(self);
    Py_XDECREF(msg);
    Py_XDECREF(writeResult);
    Py_RETURN_NONE;
error:
    Handler_handleError(&self->handler, args[0]);
    Py_XDECREF(msg);
    return nullptr;
}

PyObject* StreamHandler_setStream(StreamHandler* self, PyObject* stream){
    // If stream would be unchanged, do nothing and return None
    if (self->stream == stream) {
        Py_RETURN_NONE;
    }
    // Otherwise flush current stream
    PyObject* result = self->stream;
    flush(self);
    Py_XDECREF(self->stream);
    // And set new stream
    self->stream = stream;
    Py_INCREF(self->stream);
    self->stream_has_flush = (PyObject_HasAttr(self->stream, self->_const_flush) == 1);
    // Return previous stream (now flushed)
    return result;
}

PyObject* StreamHandler_flush(StreamHandler* self, PyObject* const* args, Py_ssize_t nargs) {
    flush(self);
    Py_RETURN_NONE;
}

PyObject* StreamHandler_repr(StreamHandler *self)
{
    std::string level = _getLevelName(self->handler.level);
    PyObject* streamName = PyObject_GetAttrString(self->stream, "name");
    PyObject* repr = PyUnicode_FromFormat("<%s %S (%s)>",
        _PyType_Name(Py_TYPE(self)),
        streamName,
        level.c_str());
    Py_CLEAR(streamName);
    return repr;
}

static PyMethodDef StreamHandler_methods[] = {
     {"emit", (PyCFunction)StreamHandler_emit, METH_FASTCALL, "Emit a record."},
     {"flush", (PyCFunction)StreamHandler_flush, METH_FASTCALL, "Flush the stream."},
     {"setStream", (PyCFunction)StreamHandler_setStream, METH_O, "Set the stream to write to."},
     {NULL}
};

static PyMemberDef StreamHandler_members[] = {
    {"stream", T_OBJECT_EX, offsetof(StreamHandler, stream), 0, "Stream"},
    {NULL}
};

PyTypeObject StreamHandlerType = {
    PyObject_HEAD_INIT(NULL)
    "picologging.StreamHandler",                    /* tp_name */
    sizeof(StreamHandler),                          /* tp_basicsize */
    0,                                          /* tp_itemsize */
    (destructor)StreamHandler_dealloc,                /* tp_dealloc */
    0,                                          /* tp_vectorcall_offset */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_as_async */
   (reprfunc)StreamHandler_repr,                /* tp_repr */
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
    PyDoc_STR("StreamHandler interface."),    /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    StreamHandler_methods,                          /* tp_methods */
    StreamHandler_members,                          /* tp_members */
    0,                                          /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    (initproc)StreamHandler_init,                   /* tp_init */
    0,                                          /* tp_alloc */
    StreamHandler_new,                          /* tp_new */
    PyObject_Del,                               /* tp_free */
};

