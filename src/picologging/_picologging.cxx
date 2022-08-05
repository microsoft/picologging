
// Python includes
#include <Python.h>

// STD includes
#include <stdio.h>
#include <unordered_map>
#include "picologging.hxx"
#include "logrecord.hxx"
#include "formatter.hxx"
#include "formatstyle.hxx"
#include "logger.hxx"
#include "handler.hxx"
#include "streamhandler.hxx"

const std::unordered_map<short, std::string> LEVELS_TO_NAMES = {
  {LOG_LEVEL_DEBUG, "DEBUG"},
  {LOG_LEVEL_INFO, "INFO"},
  {LOG_LEVEL_WARNING, "WARNING"},
  {LOG_LEVEL_ERROR, "ERROR"},
  {LOG_LEVEL_CRITICAL, "CRITICAL"},
  {LOG_LEVEL_NOTSET, "NOTSET"},
};

const std::unordered_map<std::string, short> NAMES_TO_LEVELS = {
  {"DEBUG", LOG_LEVEL_DEBUG},
  {"INFO", LOG_LEVEL_INFO},
  {"WARNING", LOG_LEVEL_WARNING},
  {"ERROR", LOG_LEVEL_ERROR},
  {"CRITICAL", LOG_LEVEL_CRITICAL},
  {"NOTSET", LOG_LEVEL_NOTSET},
};

std::string _getLevelName(short level) {
  std::unordered_map<short, std::string>::const_iterator it;
  it = LEVELS_TO_NAMES.find(level);

  if (it == LEVELS_TO_NAMES.end()){
    return "";
  }

  return it->second;
}

short _getLevelByName(std::string levelName) {
  std::unordered_map<std::string, short>::const_iterator it;
  it = NAMES_TO_LEVELS.find(levelName);

  if (it == NAMES_TO_LEVELS.end()){
    return -1;
  }

  return it->second;
}

static PyObject *getLevelName(PyObject *self, PyObject *level) {
  if (PyLong_Check(level)) {
    short levelValue = (short)PyLong_AsLong(level);
    std::string levelName = _getLevelName(levelValue);
    if (levelName.length() > 0) {
      return PyUnicode_FromString(levelName.c_str());
    }
    PyErr_Format(PyExc_ValueError, "Invalid level value: %d", levelValue);
    return nullptr;
  }

  if (PyUnicode_Check(level)) {
    short levelValue = _getLevelByName(PyUnicode_AsUTF8(level));
    if (levelValue >= 0) {
      return PyLong_FromLong(levelValue);
    }
    PyErr_Format(PyExc_ValueError, "Invalid level value: %U", level);
    return nullptr;
  }

  PyErr_SetString(PyExc_TypeError, "level must be an integer or a string.");
  return NULL;
}

//-----------------------------------------------------------------------------
static PyMethodDef picologging_methods[] = {
  {"getLevelName", (PyCFunction)getLevelName, METH_O, "Get level name by level number."},
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
  if (PyType_Ready(&FormatStyleType) < 0)
    return NULL;
  if (PyType_Ready(&FormatterType) < 0)
    return NULL;
  if (PyType_Ready(&FiltererType) < 0)
    return NULL;

  LoggerType.tp_base = &FiltererType;
  if (PyType_Ready(&LoggerType) < 0)
    return NULL;

  HandlerType.tp_base = &FiltererType;
  if (PyType_Ready(&HandlerType) < 0)
    return NULL;
  
  StreamHandlerType.tp_base = &HandlerType;
  if (PyType_Ready(&StreamHandlerType) < 0)
    return NULL;
  
  PyObject* m = PyModule_Create(&_picologging_module);
  if (m == NULL)
    return NULL;

  Py_INCREF(&LogRecordType);
  Py_INCREF(&FormatStyleType);
  Py_INCREF(&FormatterType);
  Py_INCREF(&FiltererType);
  Py_INCREF(&LoggerType);
  Py_INCREF(&HandlerType);
  Py_INCREF(&StreamHandlerType);
    
  if (PyModule_AddObject(m, "LogRecord", (PyObject *)&LogRecordType) < 0){
    Py_DECREF(&LogRecordType);
    Py_DECREF(m);
    return NULL;
  }
  if (PyModule_AddObject(m, "FormatStyle", (PyObject *)&FormatStyleType) < 0){
    Py_DECREF(&FormatStyleType);
    Py_DECREF(m);
    return NULL;
  }
  if (PyModule_AddObject(m, "Formatter", (PyObject *)&FormatterType) < 0){
    Py_DECREF(&FormatterType);
    Py_DECREF(m);
    return NULL;
  }
  if (PyModule_AddObject(m, "Filterer", (PyObject *)&FiltererType) < 0){
    Py_DECREF(&FiltererType);
    Py_DECREF(m);
    return NULL;
  }
  if (PyModule_AddObject(m, "Logger", (PyObject *)&LoggerType) < 0){
    Py_DECREF(&LoggerType);
    Py_DECREF(m);
    return NULL;
  }
  if (PyModule_AddObject(m, "Handler", (PyObject *)&HandlerType) < 0){
    Py_DECREF(&HandlerType);
    Py_DECREF(m);
    return NULL;
  }
  if (PyModule_AddObject(m, "StreamHandler", (PyObject *)&StreamHandlerType) < 0){
    Py_DECREF(&StreamHandlerType);
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

  PyObject* traceback = PyImport_ImportModule("traceback");
  if (traceback == NULL)
    return NULL;
  PyObject* print_exception = PyObject_GetAttrString(traceback, "print_exception");
  if (print_exception == NULL)
    return NULL;
  PyObject* print_stack = PyObject_GetAttrString(traceback, "print_stack");
  if (print_stack == NULL)
    return NULL;
  Py_DECREF(traceback);
  if (PyModule_AddObject(m, "print_exception", print_exception) < 0){
    Py_DECREF(print_exception);
    Py_DECREF(m);
    return NULL;
  }
  if (PyModule_AddObject(m, "print_stack", print_stack) < 0){
    Py_DECREF(print_stack);
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
