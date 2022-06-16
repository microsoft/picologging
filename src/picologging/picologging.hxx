#include <Python.h>

#ifndef PICOLOGGING_H
#define PICOLOGGING_H

extern struct PyModuleDef _picologging_module;

#define PICOLOGGING_MODULE() PyState_FindModule(&_picologging_module)

#endif // PICOLOGGING_H