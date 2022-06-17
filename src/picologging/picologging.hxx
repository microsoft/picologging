#include <Python.h>

#ifndef PICOLOGGING_H
#define PICOLOGGING_H

extern struct PyModuleDef _picologging_module;

#define PICOLOGGING_MODULE() PyState_FindModule(&_picologging_module)

#define LOG_LEVEL_CRITICAL 50
#define LOG_LEVEL_ERROR 40
#define LOG_LEVEL_WARNING 30
#define LOG_LEVEL_INFO 20
#define LOG_LEVEL_DEBUG 10
#define LOG_LEVEL_NOTSET 0

#endif // PICOLOGGING_H