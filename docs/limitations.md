# Limitations

## Formatter Interface

* Overriding `.formatStack()` is not supported
* Formatting any object other than `picologging.LogRecord` is not supported

## LogRecord

* Process name is not captured, `processName` will always be None
* Thread name is not captured, `threadName` will always be None
* LogRecord does not observe the `logging.logThreads`, `logging.logMultiprocessing`, or `logging.logProcesses` globals. It will always capture these values

## Logger

* Logger will always default to the Sys.stderr and not observe (emittedNoHandlerWarning).
