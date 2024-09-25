#include <filesystem>
#include "filepathcache.hxx"

namespace fs = std::filesystem;

PyObject* lookup(PyObject* cache, PyObject* pathname){
    PyObject* result = PyDict_GetItem(cache, pathname);
    if (result != NULL){
        return result;
    }

    fs::path fs_path = fs::path(PyUnicode_AsUTF8(pathname));
    PyObject *filename = nullptr, *module = nullptr;
#ifdef WIN32
    const wchar_t* filename_wchar = fs_path.filename().c_str();
    const wchar_t* modulename = fs_path.stem().c_str();
    filename = PyUnicode_FromWideChar(filename_wchar, wcslen(filename_wchar)),
    module = PyUnicode_FromWideChar(modulename, wcslen(modulename));
#else
    filename = PyUnicode_FromString(fs_path.filename().c_str());
    module = PyUnicode_FromString(fs_path.stem().c_str());
#endif
    PyObject *cacheItem = PyTuple_Pack(2, filename, module);
    PyDict_SetItem(cache, pathname, cacheItem);
    Py_DECREF(cacheItem);
    Py_DECREF(filename);
    Py_DECREF(module);
    return cacheItem;
}
