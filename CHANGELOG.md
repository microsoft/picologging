# Changelog

## 0.2.0 (main)

* Adds `.close()`, `.flush()`, `.createLock()` abstract methods to Handler https://github.com/microsoft/picologging/pull/9
* Corrected type stubs for Handler `__init__`
* Added simple `handleError` method to base Handler type to print the exception on sys.stderr
* Added `.get_name()` and `.set_name()` methods to Handler
* Fixes a bug in stream handler swallowing error message when `.write()` failed on the underlying stream
* Repeat all tests and isolate reference bugs in formatter and handler https://github.com/microsoft/picologging/pull/12
* Fix root logger instantiation with wrong arguments https://github.com/microsoft/picologging/pull/15
* Fix getlevelname missing from module https://github.com/microsoft/picologging/pull/16
* Fixes StreamHandler not defaulting to sys.stderr when stream argument is None https://github.com/microsoft/picologging/pull/18

## 0.1.0 (22nd June 2022)

* Initial release
* Handler base class
* Stream Handler support
