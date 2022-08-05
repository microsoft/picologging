.. _limitations:

Limitations
===========

Formatter Interface
-------------------

* Overriding `.formatStack()` is not supported
* Formatting any object other than `picologging.LogRecord` is not supported

LogRecord
---------

* Process name is not captured, `processName` will always be None (Process ID is captured).
* Thread name is not captured, `threadName` will always be None. (Thread ID is captured).
* LogRecord does not observe the `logging.logThreads`, `logging.logMultiprocessing`, or `logging.logProcesses` globals. It will *always* capture process and thread ID because the check is slower than the capture.

Logger
------

* Custom logging levels are not supported.
* There is no Log Record Factory, picologging will always use LogRecord.
* Logger will always default to the Sys.stderr and not observe (emittedNoHandlerWarning).
* Having a logger with level NOTSET and a parent is supported, but changing the level of the parent *after* setting the parent field of the child will not propagate the level to the parent. See `#22 <https://github.com/microsoft/picologging/issues/22>`_.
