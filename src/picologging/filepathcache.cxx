#include <filesystem>
#include "filepathcache.hxx"

namespace fs = std::filesystem;

PyObject* lookup(PyObject* cache, PyObject* pathname){
    PyObject* result = PyDict_GetItem(cache, pathname);
    if (result != NULL){
        return result; // todo : decide on refcnt.
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
    PyObject *cacheItem = PyTuple_Pack(2, module, filename);
    PyDict_SetItem(cache, pathname, cacheItem);
    // TODO : I think this needs a decref to set the tuple total refcnt to 1 
    // Otherwise it won't get GC'ed when this module is unloaded (which is when Python is shutdown so it doesn't matter much)
    return cacheItem;
}
