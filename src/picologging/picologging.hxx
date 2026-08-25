#include <string>
#include <Python.h>
#include "filepathcache.hxx"

#ifndef PICOLOGGING_H
#define PICOLOGGING_H

typedef struct {
  PyObject* g_filepathCache;
  PyObject* g_const_CRITICAL;
  PyObject* g_const_ERROR;
  PyObject* g_const_WARNING;
  PyObject* g_const_INFO;
  PyObject* g_const_DEBUG;
  PyObject* g_const_NOTSET;
} picologging_state;

extern struct PyModuleDef _picologging_module;
std::string _getLevelName(short);
short getLevelByName(std::string levelName);

#define PICOLOGGING_MODULE() PyState_FindModule(&_picologging_module)
#define GET_PICOLOGGING_STATE() (picologging_state *)PyModule_GetState(PICOLOGGING_MODULE())

#define LOG_LEVEL_CRITICAL 50
#define LOG_LEVEL_ERROR 40
#define LOG_LEVEL_WARNING 30
#define LOG_LEVEL_INFO 20
#define LOG_LEVEL_DEBUG 10
#define LOG_LEVEL_NOTSET 0

#endif // PICOLOGGING_H