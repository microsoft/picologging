#include <Python.h>
#include <structmember.h>
#include <cstddef>
#include <vector>

#ifndef PICOLOGGING_FILEPATHCACHE_H
#define PICOLOGGING_FILEPATHCACHE_H

typedef struct {
    PyObject* filename;
    PyObject* module;
} FilepathCacheEntry;

class FilepathCache {
    std::vector<std::pair<Py_hash_t, FilepathCacheEntry>> cache;
public:
    const FilepathCacheEntry& lookup(PyObject* filepath);
    ~FilepathCache();
};

#endif // PICOLOGGING_FILEPATHCACHE_H