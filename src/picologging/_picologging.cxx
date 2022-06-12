
// Python includes
#include <Python.h>

// STD includes
#include <stdio.h>

#include "logrecord.h"
#include "formatter.h"
#include "formatstyle.h"

//-----------------------------------------------------------------------------
static PyMethodDef picologging_methods[] = {
  {NULL, NULL, 0, NULL}        /* Sentinel */
};

//-----------------------------------------------------------------------------

static struct PyModuleDef picologging_module_def = {
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

  PyObject* m = PyModule_Create(&picologging_module_def);
  if (m == NULL)
    return NULL;

  Py_INCREF(&LogRecordType);
  Py_INCREF(&PercentStyleType);
  Py_INCREF(&FormatterType);
    
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

  return m;
}
