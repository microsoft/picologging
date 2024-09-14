import io
import os
import sys
import warnings
from logging import BufferingFormatter, Filter, StringTemplateStyle, _checkLevel  # NOQA

from ._picologging import Handler  # NOQA
from ._picologging import (  # NOQA
    Filterer,
    FormatStyle,
    Formatter,
    Logger,
    LogRecord,
    StreamHandler,
    getLevelName,
)

__version__ = "0.9.4"

CRITICAL = 50
FATAL = CRITICAL
ERROR = 40
WARNING = 30
WARN = WARNING
INFO = 20
DEBUG = 10
NOTSET = 0

BASIC_FORMAT = "%(levelname)s:%(name)s:%(message)s"


if hasattr(io, "text_encoding"):
    text_encoding = io.text_encoding
else:

    def text_encoding(encoding) -> str:
        if encoding is not None:
            return encoding
        if sys.flags.utf8_mode:
            return "utf-8"
        return "locale"


class PercentStyle(FormatStyle):
    def __new__(cls, *args, **kwargs):
        kwargs["style"] = "%"
        return super().__new__(cls, *args, **kwargs)

    def __init__(self, fmt, defaults=None):
        super().__init__(fmt, defaults, style="%")


class StrFormatStyle(FormatStyle):
    def __new__(cls, *args, **kwargs):
        kwargs["style"] = "{"
        return super().__new__(cls, *args, **kwargs)

    def __init__(self, fmt, defaults=None):
        super().__init__(fmt, defaults, style="{")


_STYLES = {
    "%": (PercentStyle, BASIC_FORMAT),
    "{": (StrFormatStyle, "{levelname}:{name}:{message}"),
    "$": (StringTemplateStyle, "${levelname}:${name}:${message}"),
}


class _Placeholder:
    """
    _Placeholder instances are used in the Manager logger hierarchy to take
    the place of nodes for which no loggers have been defined. This class is
    intended for internal use only and not as part of the public API.
    """

    def __init__(self, alogger):
        """
        Initialize with the specified logger being a child of this placeholder.
        """
        self.loggerMap = {alogger: None}

    def append(self, alogger):
        """
        Add the specified logger as a child of this placeholder.
        """
        if alogger not in self.loggerMap:
            self.loggerMap[alogger] = None


class Manager:
    """
    There is [under normal circumstances] just one Manager instance, which
    holds the hierarchy of loggers.
    """

    def __init__(self, rootnode, cls=None):
        """
        Initialize the manager with the root node of the logger hierarchy.
        """
        self.root = rootnode
        self.disable = 0
        self.emittedNoHandlerWarning = False
        self.loggerDict = {}
        if not cls:
            self.cls = Logger
        else:
            self.cls = cls

    @property
    def disable(self):
        return self._disable

    @disable.setter
    def disable(self, value):
        self._disable = _checkLevel(value)

    def getLogger(self, name):
        """
        Get a logger with the specified name (channel name), creating it
        if it doesn't yet exist. This name is a dot-separated hierarchical
        name, such as "a", "a.b", "a.b.c" or similar.
        """
        if name in self.loggerDict:
            rv = self.loggerDict[name]
            if isinstance(rv, _Placeholder):
                ph = rv
                rv = self.cls(name)
                rv.manager = self
                self.loggerDict[name] = rv
                self._fixupChildren(ph, rv)
                self._fixupParents(rv)
        else:
            rv = self.cls(name)
            rv.manager = self
            self.loggerDict[name] = rv
            self._fixupParents(rv)
        return rv

    def _fixupParents(self, alogger):
        """
        Ensure that there are either loggers or placeholders all the way
        from the specified logger to the root of the logger hierarchy.
        """
        name = alogger.name
        i = name.rfind(".")
        logger_parent = None
        while (i > 0) and not logger_parent:
            substr = name[:i]
            if substr not in self.loggerDict:
                self.loggerDict[substr] = _Placeholder(alogger)
            else:
                obj = self.loggerDict[substr]
                if isinstance(obj, Logger):
                    logger_parent = obj
                else:
                    assert isinstance(obj, _Placeholder)
                    obj.append(alogger)
            i = name.rfind(".", 0, i - 1)
        if not logger_parent:
            logger_parent = self.root
        alogger.parent = logger_parent

    def _fixupChildren(self, ph, alogger):
        """
        Ensure that children of the placeholder ph are connected to the
        specified logger.
        """
        name = alogger.name
        namelen = len(name)
        for c in ph.loggerMap.keys():
            # The if means ... if not c.parent.name.startswith(nm)
            if c.parent.name[:namelen] != name:
                alogger.parent = c.parent
                c.parent = alogger

    def setLoggerClass(self, klass):
        self.cls = klass

    def setLogRecordFactory(self, factory):
        raise NotImplementedError(
            "setLogRecordFactory is not supported in picologging."
        )


root = Logger(name="root", level=WARNING)
root.manager = Manager(root)


def basicConfig(**kwargs):
    """
    Do basic configuration for the logging system.

    This function does nothing if the root logger already has handlers
    configured, unless the keyword argument *force* is set to ``True``.
    It is a convenience method intended for use by simple scripts
    to do one-shot configuration of the logging package.

    The default behaviour is to create a StreamHandler which writes to
    sys.stderr, set a formatter using the BASIC_FORMAT format string, and
    add the handler to the root logger.

    A number of optional keyword arguments may be specified, which can alter
    the default behaviour.

    filename  Specifies that a FileHandler be created, using the specified
              filename, rather than a StreamHandler.
    filemode  Specifies the mode to open the file, if filename is specified
              (if filemode is unspecified, it defaults to 'a').
    format    Use the specified format string for the handler.
    datefmt   Use the specified date/time format.
    style     If a format string is specified, use this to specify the
              type of format string (possible values '%', '{', '$', for
              %-formatting, :meth:`str.format` and :class:`string.Template`
              - defaults to '%').
    level     Set the root logger level to the specified level.
    stream    Use the specified stream to initialize the StreamHandler. Note
              that this argument is incompatible with 'filename' - if both
              are present, 'stream' is ignored.
    handlers  If specified, this should be an iterable of already created
              handlers, which will be added to the root handler. Any handler
              in the list which does not have a formatter assigned will be
              assigned the formatter created in this function.
    force     If this keyword  is specified as true, any existing handlers
              attached to the root logger are removed and closed, before
              carrying out the configuration as specified by the other
              arguments.
    encoding  If specified together with a filename, this encoding is passed to
              the created FileHandler, causing it to be used when the file is
              opened.
    errors    If specified together with a filename, this value is passed to the
              created FileHandler, causing it to be used when the file is
              opened in text mode. If not specified, the default value is
              `backslashreplace`.

    Note that you could specify a stream created using open(filename, mode)
    rather than passing the filename and mode in. However, it should be
    remembered that StreamHandler does not close its stream (since it may be
    using sys.stdout or sys.stderr), whereas FileHandler closes its stream
    when the handler is closed.

    .. versionchanged:: 3.2
       Added the ``style`` parameter.

    .. versionchanged:: 3.3
       Added the ``handlers`` parameter. A ``ValueError`` is now thrown for
       incompatible arguments (e.g. ``handlers`` specified together with
       ``filename``/``filemode``, or ``filename``/``filemode`` specified
       together with ``stream``, or ``handlers`` specified together with
       ``stream``.

    .. versionchanged:: 3.8
       Added the ``force`` parameter.

    .. versionchanged:: 3.9
       Added the ``encoding`` and ``errors`` parameters.
    """
    # Add thread safety in case someone mistakenly calls
    # basicConfig() from multiple threads
    force = kwargs.pop("force", False)
    encoding = kwargs.pop("encoding", None)
    errors = kwargs.pop("errors", "backslashreplace")
    if force:
        for h in root.handlers[:]:
            root.removeHandler(h)
            h.close()
    if len(root.handlers) == 0:
        handlers = kwargs.pop("handlers", None)
        if handlers is None:
            if "stream" in kwargs and "filename" in kwargs:
                raise ValueError(
                    "'stream' and 'filename' should not be " "specified together"
                )
        else:
            if "stream" in kwargs or "filename" in kwargs:
                raise ValueError(
                    "'stream' or 'filename' should not be "
                    "specified together with 'handlers'"
                )
        if handlers is None:
            filename = kwargs.pop("filename", None)
            mode = kwargs.pop("filemode", "a")
            if filename:
                if "b" in mode:
                    errors = None
                else:
                    encoding = text_encoding(encoding)
                h = FileHandler(filename, mode, encoding=encoding, errors=errors)
            else:
                stream = kwargs.pop("stream", sys.stderr)
                h = StreamHandler(stream)
            handlers = [h]
        dfs = kwargs.pop("datefmt", None)
        style = kwargs.pop("style", "%")
        if style not in _STYLES:
            raise ValueError("Style must be one of: %s" % ",".join(_STYLES.keys()))
        fs = kwargs.pop("format", _STYLES[style][1])
        fmt = Formatter(fs, dfs, style)
        for h in handlers:
            if h.formatter is None:
                h.setFormatter(fmt)
            root.addHandler(h)
        level = kwargs.pop("level", None)
        if level is not None:
            root.setLevel(level)
        if kwargs:
            keys = ", ".join(kwargs.keys())
            raise ValueError("Unrecognised argument(s): %s" % keys)


def getLogger(name=None):
    """
    Return a logger with the specified name, creating it if necessary.

    If no name is specified, return the root logger.
    """
    if not name or isinstance(name, str) and name == root.name:
        return root
    return root.manager.getLogger(name)


def critical(msg, *args, **kwargs):
    """
    Log a message with severity 'CRITICAL' on the root logger. If the logger
    has no handlers, call basicConfig() to add a console handler with a
    pre-defined format.
    """
    if len(root.handlers) == 0:
        basicConfig()
    root.critical(msg, *args, **kwargs)


def fatal(msg, *args, **kwargs):
    """
    Don't use this function, use critical() instead.
    """
    critical(msg, *args, **kwargs)


def error(msg, *args, **kwargs):
    """
    Log a message with severity 'ERROR' on the root logger. If the logger has
    no handlers, call basicConfig() to add a console handler with a pre-defined
    format.
    """
    if len(root.handlers) == 0:
        basicConfig()
    root.error(msg, *args, **kwargs)


def exception(msg, *args, exc_info=True, **kwargs):
    """
    Log a message with severity 'ERROR' on the root logger, with exception
    information. If the logger has no handlers, basicConfig() is called to add
    a console handler with a pre-defined format.
    """
    error(msg, *args, exc_info=exc_info, **kwargs)


def warning(msg, *args, **kwargs):
    """
    Log a message with severity 'WARNING' on the root logger. If the logger has
    no handlers, call basicConfig() to add a console handler with a pre-defined
    format.
    """
    if len(root.handlers) == 0:
        basicConfig()
    root.warning(msg, *args, **kwargs)


def warn(msg, *args, **kwargs):
    warnings.warn(
        "The 'warn' function is deprecated, " "use 'warning' instead",
        DeprecationWarning,
        2,
    )
    warning(msg, *args, **kwargs)


def info(msg, *args, **kwargs):
    """
    Log a message with severity 'INFO' on the root logger. If the logger has
    no handlers, call basicConfig() to add a console handler with a pre-defined
    format.
    """
    if len(root.handlers) == 0:
        basicConfig()
    root.info(msg, *args, **kwargs)


def debug(msg, *args, **kwargs):
    """
    Log a message with severity 'DEBUG' on the root logger. If the logger has
    no handlers, call basicConfig() to add a console handler with a pre-defined
    format.
    """
    if len(root.handlers) == 0:
        basicConfig()
    root.debug(msg, *args, **kwargs)


def log(level, msg, *args, **kwargs):
    """
    Log 'msg % args' with the integer severity 'level' on the root logger. If
    the logger has no handlers, call basicConfig() to add a console handler
    with a pre-defined format.
    """
    if len(root.handlers) == 0:
        basicConfig()
    root.log(level, msg, *args, **kwargs)


def disable(level=CRITICAL):
    """
    Disable all logging calls of severity 'level' and below.
    """
    root.manager.disable = level
    root.manager._clear_cache()


class NullHandler(Handler):
    """
    This handler does nothing. It's intended to be used to avoid the
    "No handlers could be found for logger XXX" one-off warning. This is
    important for library code, which may contain code to log events. If a user
    of the library does not configure logging, the one-off warning might be
    produced; to avoid this, the library developer simply needs to instantiate
    a NullHandler and add it to the top-level logger of the library module or
    package.
    """

    def handle(self, record):
        """Stub."""

    def emit(self, record):
        """Stub."""


class FileHandler(StreamHandler):
    """
    A handler class which writes formatted logging records to disk files.
    """

    def __init__(self, filename, mode="a", encoding=None, delay=False, errors=None):
        """
        Open the specified file and use it as the stream for logging.
        """
        # Issue #27493: add support for Path objects to be passed in
        filename = os.fspath(filename)
        # keep the absolute path, otherwise derived classes which use this
        # may come a cropper when the current directory changes
        self.baseFilename = os.path.abspath(filename)
        self.mode = mode
        self.encoding = encoding
        self.delay = delay
        self.errors = errors
        if delay:
            # We don't open the stream, but we still need to call the
            # Handler constructor to set level, formatter, lock etc.
            Handler.__init__(self)
            self.stream = None
        else:
            StreamHandler.__init__(self, self._open())

    def close(self):
        """
        Closes the stream.
        """
        self.acquire()
        try:
            try:
                if self.stream:
                    try:
                        self.flush()
                    finally:
                        stream = self.stream
                        self.stream = None
                        if hasattr(stream, "close"):
                            stream.close()
            finally:
                # Issue #19523: call unconditionally to
                # prevent a handler leak when delay is set
                StreamHandler.close(self)
        finally:
            self.release()

    def _open(self):
        """
        Open the current base file with the (original) mode and encoding.
        Return the resulting stream.
        """
        return open(
            self.baseFilename, self.mode, encoding=self.encoding, errors=self.errors
        )

    def emit(self, record):
        """
        Emit a record.

        If the stream was not opened because 'delay' was specified in the
        constructor, open it before calling the superclass's emit.
        """
        if self.stream is None:
            self.stream = self._open()
        StreamHandler.emit(self, record)

    def __repr__(self):
        level = getLevelName(self.level)
        return f"<{self.__class__.__name__} {self.baseFilename} ({level})>"


def makeLogRecord(dict):
    """
    Make a LogRecord whose attributes are defined by the specified dictionary,
    This function is useful for converting a logging event received over
    a socket connection (which is sent as a dictionary) into a LogRecord
    instance.
    """

    rv = LogRecord("", NOTSET, "", 0, "", None, None)
    for k, v in dict.items():
        setattr(rv, k, v)
    return rv
