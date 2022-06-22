# Changelog

## 0.2.0 (main)

* Adds `.close()`, `.flush()`, `.createLock()` abstract methods to Handler
* Corrected type stubs for Handler `__init__`
* Added simple `handleError` method to base Handler type to print the exception on sys.stderr
* Added `.get_name()` and `.set_name()` methods to Handler
* Fixes a bug in stream handler swallowing error message when `.write()` failed on the underlying stream

## 0.1.0 (22nd June 2022)

* Initial release
* Handler base class
* Stream Handler support
