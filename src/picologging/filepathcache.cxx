#include <filesystem>
#include "filepathcache.hxx"

namespace fs = std::filesystem;

const FilepathCacheEntry& FilepathCache::lookup(PyObject* pathname){
    /*
     * Notes: A vector ended up being significantly faster than an unordered_map,
     * even though an unordered_map should probably be used.
     * TODO #3 : Cap vector size or decide on a better map type.
     */
    Py_hash_t hash = PyObject_Hash(pathname);
    for (auto& entry : cache){
        if (entry.first == hash){
            return entry.second;
        }
    }
    FilepathCacheEntry* entry = new FilepathCacheEntry();
    fs::path fs_path = fs::path(PyUnicode_AsUTF8(pathname));
#ifdef WIN32
    const wchar_t* filename_wchar = fs_path.filename().c_str();
    const wchar_t* modulename = fs_path.stem().c_str();
    entry->filename = PyUnicode_FromWideChar(filename_wchar, wcslen(filename_wchar)),
    entry->module = PyUnicode_FromWideChar(modulename, wcslen(modulename));
#else
    entry->filename = PyUnicode_FromString(fs_path.filename().c_str());
    entry->module = PyUnicode_FromString(fs_path.stem().c_str());
#endif
    cache.push_back({hash, *entry});
    return *entry;
}

FilepathCache::~FilepathCache(){
    for (auto& entry : cache){
        Py_CLEAR(entry.second.filename);
        Py_CLEAR(entry.second.module);
    }
}