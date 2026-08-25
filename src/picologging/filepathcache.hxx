#include <Python.h>
#include <structmember.h>
#include <cstddef>

#ifndef PICOLOGGING_FILEPATHCACHE_H
#define PICOLOGGING_FILEPATHCACHE_H

PyObject* lookup(PyObject* cache, PyObject* pathname);
#endif // PICOLOGGING_FILEPATHCACHE_H