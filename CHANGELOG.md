# Changelog

## 0.9.4

* Improve the performance of logging records by inlining log creation to remove temporary Python objects into the logger. by @tonybaloney in https://github.com/microsoft/picologging/pull/186
* Improve the performance of logger methods with fastcalls by @tonybaloney in https://github.com/microsoft/picologging/pull/188
* Improve perf and memory usage of log records with const level names by @tonybaloney in https://github.com/microsoft/picologging/pull/190
* Add example using picologging and django by @tonybaloney in https://github.com/microsoft/picologging/pull/194
* Fix setLogRecordFactory exception message by @GabrielCappelli in https://github.com/microsoft/picologging/pull/196
* Fix asctime formatting, don't print uninitialized buffer's content (issue #203) by @tarasko in https://github.com/microsoft/picologging/pull/208
* Add a test for issue Using `asctime` in formatter by @tonybaloney in https://github.com/microsoft/picologging/pull/210

## 0.9.3

* Support for Python 3.12
* Module can be used in sub-interpreters (3.12+)
* Allow LogRecord to be copied by @aminalaee in https://github.com/microsoft/picologging/pull/164

### Bug fixes

* Fix memory leak in LogRecord.__dict__. Use Py_NewRef and Py_CLEAR patterns for cleaner code by @tonybaloney in https://github.com/microsoft/picologging/pull/173
* Fix `io` text_encoding on Python 3.9 and older by @aminalaee in https://github.com/microsoft/picologging/pull/152
* Add tests for override formatter formatException by @aminalaee in https://github.com/microsoft/picologging/pull/140
* Fix formatting issues by @aminalaee in https://github.com/microsoft/picologging/pull/160
* Add FileHandler errors argument by @aminalaee in https://github.com/microsoft/picologging/pull/161
* Move filepath cache to within free-able module state by @tonybaloney in https://github.com/microsoft/picologging/pull/166
* Bump pypa/cibuildwheel from 2.12.3 to 2.15.0 by @dependabot in https://github.com/microsoft/picologging/pull/158
* Use a module state attribute to support subinterpreters by @tonybaloney in https://github.com/microsoft/picologging/pull/167
* Cleanup all danging unicode references. by @tonybaloney in https://github.com/microsoft/picologging/pull/169
* Adopt the memray trace leaks test markers by @tonybaloney in https://github.com/microsoft/picologging/pull/137
* Fix unclosed resources in tests by @aminalaee in https://github.com/microsoft/picologging/pull/171
* Verify the integration of coloredlogs is fixed by @tonybaloney in https://github.com/microsoft/picologging/pull/174

## 0.9.2

* Upgrade pre-commit hooks versions by @sadikkuzu in https://github.com/microsoft/picologging/pull/133
* Compile x86 for linux and windows. Compile aarch64 for linux by @tonybaloney in https://github.com/microsoft/picologging/pull/135
* moves devcontainer vscode settings by @kjaymiller in https://github.com/microsoft/picologging/pull/143
* Fix Formatter __repr__ by @aminalaee in https://github.com/microsoft/picologging/pull/141
* Fix leaks in __dict__ and asctime fields by @tonybaloney in https://github.com/microsoft/picologging/pull/145
* Add Logger isEnabledFor by @aminalaee in https://github.com/microsoft/picologging/pull/139
* Support Logger.setLevel from string input by @tonybaloney in https://github.com/microsoft/picologging/pull/146

## 0.9.1

* Add Windows Arm64 wheels by @aminalaee in https://github.com/microsoft/picologging/pull/130
* Remove #22 as limitation since it's been fixed by @tonybaloney in https://github.com/microsoft/picologging/pull/128
* Remove QueueListener test by @aminalaee in https://github.com/microsoft/picologging/pull/101

## 0.9.0

* Fix formatting with black by @aminalaee in https://github.com/microsoft/picologging/pull/71
* Improve test coverage by @tonybaloney in https://github.com/microsoft/picologging/pull/72
* Fix FileHandler flaky test by @aminalaee in https://github.com/microsoft/picologging/pull/69
* Update project classifiers by @aminalaee in https://github.com/microsoft/picologging/pull/74
* Uplift test coverage by @aminalaee in https://github.com/microsoft/picologging/pull/76
* Making LogRecord class derivable by @pamelafox in https://github.com/microsoft/picologging/pull/77
* Properly raise error from Handler.emit by @pamelafox in https://github.com/microsoft/picologging/pull/81
* Properly handle return value for setStream by @pamelafox in https://github.com/microsoft/picologging/pull/82
* Fixing Stream.__repr__ to match CPython by @pamelafox in https://github.com/microsoft/picologging/pull/84
* Add Handler repr by @aminalaee in https://github.com/microsoft/picologging/pull/88
* Allow handle method to accept LogRecord subclasses by @aminalaee in https://github.com/microsoft/picologging/pull/85
* Fix Handler.handle return type by @aminalaee in https://github.com/microsoft/picologging/pull/86
* Make config fix by @pamelafox in https://github.com/microsoft/picologging/pull/97
* Add failing Queue Listener Handler Test by @Goldziher in https://github.com/microsoft/picologging/pull/89
* Add local development instructions to readme by @pamelafox in https://github.com/microsoft/picologging/pull/104
* Add launch.json instructions to README by @pamelafox in https://github.com/microsoft/picologging/pull/106
* Create py.typed by @Goldziher in https://github.com/microsoft/picologging/pull/92
* Add config.pyi stub by @aminalaee in https://github.com/microsoft/picologging/pull/100
* Add pylint config by @pamelafox in https://github.com/microsoft/picologging/pull/107
* 3.7+ compatible code by @tonybaloney in https://github.com/microsoft/picologging/pull/114
* Document the memray tests and add a script for automation by @tonybaloney in https://github.com/microsoft/picologging/pull/113
* Adding regression test for LogRecord args issue by @pamelafox in https://github.com/microsoft/picologging/pull/105
* Fix leaks identified memray by @tonybaloney in https://github.com/microsoft/picologging/pull/115
* Set logger parents and levels by @pamelafox in https://github.com/microsoft/picologging/pull/108
* Adding parameters to flaky decorator to increase reruns by @pamelafox in https://github.com/microsoft/picologging/pull/118
* Adding precommit with black and pyupgr by @pamelafox in https://github.com/microsoft/picologging/pull/119
* Call dealloc of base types by @pamelafox in https://github.com/microsoft/picologging/pull/120
* Add isort by @pamelafox in https://github.com/microsoft/picologging/pull/121
* Test on 3.11 vs 3.11-dev by @pamelafox in https://github.com/microsoft/picologging/pull/127
* Make QueueHandler call format by @pamelafox in https://github.com/microsoft/picologging/pull/122

## 0.8.1

* Fix `dictconfig` resetting child loggers by @aminalaee in https://github.com/microsoft/picologging/pull/70
* Add `formatException` method in `Formatter` by @aminalaee in https://github.com/microsoft/picologging/pull/68

## 0.8.0

* Add `dictConfig` method https://github.com/microsoft/picologging/pull/61
* Add `DatagramHandler` https://github.com/microsoft/picologging/pull/64

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
