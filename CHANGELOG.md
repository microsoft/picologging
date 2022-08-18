# Changelog

## 0.7.2

* Fix leak on levelname and formatted message https://github.com/microsoft/picologging/pull/59
* Fixes a leak in the `asctime` field of log record https://github.com/microsoft/picologging/pull/60
* Fixes a leak in stream writer write() and flush() calls https://github.com/microsoft/picologging/pull/60
* Fixes a leak in stream close() https://github.com/microsoft/picologging/pull/60

## 0.7.1

* Add  a basic documentation site by @tonybaloney in https://github.com/microsoft/picologging/pull/55
* Fix Logger setLevel not resetting other levels by @aminalaee in https://github.com/microsoft/picologging/pull/57
* Fixes reference leaks in filterer 'filter' string and relative time by @tonybaloney in https://github.com/microsoft/picologging/pull/58

## 0.7.0

* Add `MemoryHandler` by @aminalaee in https://github.com/microsoft/picologging/pull/46
* Make `getLevelName()` work level name arguments by @aminalaee in https://github.com/microsoft/picologging/pull/52
* Add `makeLogRecord()` by @aminalaee in https://github.com/microsoft/picologging/pull/50
* Add `SocketHandler` by @aminalaee in https://github.com/microsoft/picologging/pull/48

## 0.6.0

* Implements QueueListener and QueueHandler in `picologging.handlers` module for non-blocking logging by @tonybaloney in https://github.com/microsoft/picologging/pull/44
* Fix a crash on ubuntu by @tonybaloney in https://github.com/microsoft/picologging/pull/42
* Add BufferingHandler by @aminalaee in https://github.com/microsoft/picologging/pull/45
* Fixes a crash on string literal within a loop in certain cases

## 0.5.1

* Fixes a crash in 32-bit Linux wheels crashing when `validate=True` flag is used for the Formatter type

## 0.5.0

* String format `{field}` is now supported using `Formatter(style='{')`
* Logger now supports `sinfo=True` to add stack info to log messages
* Fixed a bug where formatters using the `created` field were not correctly formatted
* Fixed a bug where formatters using the thread id, `thread` field were formatted as signed int instead of unsigned long
* Fixed a bug in the `__repr__` method of PercentStyle
* Fixed a bug that the Logger.exception() method wasn't checking the log level for ERROR
* Fixed a bug in the formatting of exception messages
* Setting the parent of a logger which has a NOTSET level will now update it's logging level to adopt the next set parent's level

## 0.4.0

* Add Fuzzing and coverage configuration for Clang/GCC by @tonybaloney in https://github.com/microsoft/picologging/pull/26
* Add WatchedFileHandler by @aminalaee in https://github.com/microsoft/picologging/pull/23
* Expand test suite with error cases by @tonybaloney in https://github.com/microsoft/picologging/pull/27
* Refactor type field initializers to `__new__` by @tonybaloney in https://github.com/microsoft/picologging/pull/28
* Add tests for FileHandler delay by @aminalaee in https://github.com/microsoft/picologging/pull/24
* Add BaseRotatingHandler by @aminalaee in https://github.com/microsoft/picologging/pull/25

## 0.3.0

* Added `FileHandler` to picologging
* Fixed an issue with the `Logger.setLevel()` method not correctly resetting level flags in the logger
* Fixed a memory leak in the logging and handler types

## 0.2.0 (23rd June 2022)

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
