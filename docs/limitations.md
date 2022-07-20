# Limitations

## Formatter Interface

* Overriding `.formatStack()` is not supported
* Formatting any object other than `picologging.LogRecord` is not supported

## LogRecord

* Process name is not captured, `processName` will always be None
* Thread name is not captured, `threadName` will always be None
* LogRecord does not observe the `logging.logThreads`, `logging.logMultiprocessing`, or `logging.logProcesses` globals. It will *always* capture these values

## Logger

* Logger will always default to the Sys.stderr and not observe (emittedNoHandlerWarning).
* Having a logger with level NOTSET and a parent is supported, but changing the level of the parent *after* setting the parent field of the child will not propagate the level to the parent. See [#22](https://github.com/microsoft/picologging/issues/22)