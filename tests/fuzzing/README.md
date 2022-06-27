# Fuzz testing

Requirements: 
 - pip install atheris

Compilation:

```console
$ CC="/usr/bin/clang" CXX="/usr/bin/clang++" python setup.py build_ext --inplace --build-type Debug -DCOVERAGE=ON -DFUZZING=ON
$ ASAN_OPTIONS=detect_leaks=0 LD_PRELOAD="$(python -c "import atheris; print(atheris.path())")/asan_with_fuzzer.so" python tests/fuzzing/fuzz_atheris.py

INFO: Instrumenting picologging
INFO: Instrumenting traceback
INFO: Instrumenting linecache
INFO: Instrumenting tokenize
INFO: Instrumenting token
INFO: Instrumenting logging
INFO: Instrumenting weakref
INFO: Instrumenting _weakrefset
INFO: Instrumenting string
INFO: Instrumenting threading
INFO: Using preloaded libfuzzer
INFO: Running with entropic power schedule (0xFF, 100).
INFO: Seed: 3598005196
INFO: Loaded 1 modules   (7006 inline 8-bit counters): 7006 [0x7f9c39ecbac8, 0x7f9c39ecd626), 
INFO: Loaded 1 PC tables (7006 PCs): 7006 [0x7f9c39ecd628,0x7f9c39ee8c08), 
INFO: -max_len is not provided; libFuzzer will not generate inputs larger than 4096 bytes
INFO: A corpus is not provided, starting from an empty corpus
#2      INITED cov: 125 ft: 126 corp: 1/1b exec/s: 0 rss: 60Mb
#65536  pulse  cov: 125 ft: 126 corp: 1/1b lim: 652 exec/s: 32768 rss: 132Mb
#131072 pulse  cov: 125 ft: 126 corp: 1/1b lim: 1300 exec/s: 43690 rss: 197Mb
#200000 NEW    cov: 126 ft: 127 corp: 2/68b lim: 1990 exec/s: 40000 rss: 270Mb L: 67/67 MS: 1 InsertRepeatedBytes-
#262144 pulse  cov: 126 ft: 127 corp: 2/68b lim: 2600 exec/s: 43690 rss: 331Mb
#524288 pulse  cov: 126 ft: 127 corp: 2/68b lim: 4096 exec/s: 40329 rss: 598Mb
#1048576        pulse  cov: 126 ft: 127 corp: 2/68b lim: 4096 exec/s: 43690 rss: 693Mb
#2097152        pulse  cov: 126 ft: 127 corp: 2/68b lim: 4096 exec/s: 42799 rss: 834Mb
#4194304        pulse  cov: 126 ft: 127 corp: 2/68b lim: 4096 exec/s: 42799 rss: 1031Mb
```
