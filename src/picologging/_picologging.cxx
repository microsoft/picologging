
// Python includes
#include <Python.h>

// STD includes
#include <stdio.h>
#include "picologging.hxx"
#include "logrecord.hxx"
#include "formatter.hxx"
#include "formatstyle.hxx"
#include "logger.hxx"

//-----------------------------------------------------------------------------
static PyMethodDef picologging_methods[] = {
  {NULL, NULL, 0, NULL}        /* Sentinel */
};

//-----------------------------------------------------------------------------

struct PyModuleDef _picologging_module = {
  PyModuleDef_HEAD_INIT,
  "_picologging",
  "Internal \"_picologging\" module",
  -1,
  picologging_methods
};

PyMODINIT_FUNC PyInit__picologging(void)
{
  if (PyType_Ready(&LogRecordType) < 0)
    return NULL;
  if (PyType_Ready(&PercentStyleType) < 0)
    return NULL;
  if (PyType_Ready(&FormatterType) < 0)
    return NULL;
  if (PyType_Ready(&LoggerType) < 0)
    return NULL;
  
  PyObject* m = PyModule_Create(&_picologging_module);
  if (m == NULL)
    return NULL;

  Py_INCREF(&LogRecordType);
  Py_INCREF(&PercentStyleType);
  Py_INCREF(&FormatterType);
  Py_INCREF(&LoggerType);
    
  if (PyModule_AddObject(m, "LogRecord", (PyObject *)&LogRecordType) < 0){
    Py_DECREF(&LogRecordType);
    Py_DECREF(m);
    return NULL;
  }
  if (PyModule_AddObject(m, "PercentStyle", (PyObject *)&PercentStyleType) < 0){
    Py_DECREF(&PercentStyleType);
    Py_DECREF(m);
    return NULL;
  }
  if (PyModule_AddObject(m, "Formatter", (PyObject *)&FormatterType) < 0){
    Py_DECREF(&FormatterType);
    Py_DECREF(m);
    return NULL;
  }
  if (PyModule_AddObject(m, "Logger", (PyObject *)&LoggerType) < 0){
    Py_DECREF(&LoggerType);
    Py_DECREF(m);
    return NULL;
  }
  if (PyModule_AddStringConstant(m, "default_fmt", "%(message)s") < 0){
    Py_DECREF(m);
    return NULL;
  }
  if (PyModule_AddStringConstant(m, "default_datefmt", "%Y-%m-%d %H:%M:%S") < 0){
    Py_DECREF(m);
    return NULL;
  }
  if (PyModule_AddStringConstant(m, "default_style", "%") < 0){
    Py_DECREF(m);
    return NULL;
  }
  if (PyModule_AddStringConstant(m, "CRITICAL", "CRITICAL") < 0){
    Py_DECREF(m);
    return NULL;
  }
  if (PyModule_AddStringConstant(m, "ERROR", "ERROR") < 0){
    Py_DECREF(m);
    return NULL;
  }
  if (PyModule_AddStringConstant(m, "WARNING", "WARNING") < 0){
    Py_DECREF(m);
    return NULL;
  }
  if (PyModule_AddStringConstant(m, "INFO", "INFO") < 0){
    Py_DECREF(m);
    return NULL;
  }
  if (PyModule_AddStringConstant(m, "DEBUG", "DEBUG") < 0){
    Py_DECREF(m);
    return NULL;
  }
  if (PyModule_AddStringConstant(m, "NOTSET", "NOTSET") < 0){
    Py_DECREF(m);
    return NULL;
  }

  PyObject* traceback = PyImport_ImportModule("traceback");
  if (traceback == NULL)
    return NULL;
  PyObject* print_exception = PyObject_GetAttrString(traceback, "print_exception");
  if (print_exception == NULL)
    return NULL;
  Py_DECREF(traceback);
  if (PyModule_AddObject(m, "print_exception", print_exception) < 0){
    Py_DECREF(print_exception);
    Py_DECREF(m);
    return NULL;
  }

  PyObject* io = PyImport_ImportModule("io");
  if (io == NULL)
    return NULL;
  PyObject* stringio = PyObject_GetAttrString(io, "StringIO");
  if (stringio == NULL)
    return NULL;
  Py_DECREF(io);
  if (PyModule_AddObject(m, "StringIO", stringio) < 0){
    Py_DECREF(stringio);
    Py_DECREF(m);
    return NULL;
  }

  return m;
}
