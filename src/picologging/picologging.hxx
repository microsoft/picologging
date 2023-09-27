#include <string>
#include <Python.h>
#include "filepathcache.hxx"

#ifndef PICOLOGGING_H
#define PICOLOGGING_H

typedef struct {
  FilepathCache* g_filepathCache;
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