import logging
import sys
from ._picologging import LogRecord, PercentStyle, Formatter, Logger, Filterer, Handler  # NOQA

__version__ = "0.1.0"
__all__ = [
    "LogRecord",
    "Formatter",
    "PercentStyle",
    "install",
    "uninstall",
]

def install():
    """ Install the picologging record and logger """
    if len(logging.root.handlers) == 0:
        raise ValueError("There are no handlers configured. Make sure logging is setup first (logging.basicSetup())")

    logging.setLogRecordFactory(LogRecord)
    logging._STYLES['%'] = (PercentStyle, logging.BASIC_FORMAT)
    logging.Logger.manager.logRecordFactory = LogRecord
    for handler in logging.Logger.root.handlers:
        handler.setFormatter(Formatter(handler.formatter._fmt))
    logging._defaultFormatter = Formatter()

def uninstall():
    """ Uninstall the picologging record and logger """
    logging.setLogRecordFactory(logging.LogRecord)
    logging.Logger.manager.logRecordFactory = logging.LogRecord
    logging._STYLES['%'] = (logging.PercentStyle, logging.BASIC_FORMAT)
    for h in logging.Logger.root.handlers:
        h.setFormatter(logging.Formatter(h.formatter._fmt))
    logging._defaultFormatter = logging.Formatter()


class StreamHandler(Handler):
    """
    A handler class which writes logging records, appropriately formatted,
    to a stream. Note that this class does not close the stream, as
    sys.stdout or sys.stderr may be used.
    """

    terminator = '\n'

    def __init__(self, stream=None):
        """
        Initialize the handler.

        If stream is not specified, sys.stderr is used.
        """
        Handler.__init__(self)
        if stream is None:
            stream = sys.stderr
        self.stream = stream

    def flush(self):
        """
        Flushes the stream.
        """
        # self.acquire()
        try:
            if self.stream and hasattr(self.stream, "flush"):
                self.stream.flush()
        finally:
            self.release()

    def emit(self, record):
        """
        Emit a record.

        If a formatter is specified, it is used to format the record.
        The record is then written to the stream with a trailing newline.  If
        exception information is present, it is formatted using
        traceback.print_exception and appended to the stream.  If the stream
        has an 'encoding' attribute, it is used to determine how to do the
        output to the stream.
        """
        try:
            msg = self.format(record)
            stream = self.stream
            # issue 35046: merged two stream.writes into one.
            stream.write(msg + self.terminator)
            self.flush()
        except RecursionError:  # See issue 36272
            raise
        except Exception:
            self.handleError(record)

    def setStream(self, stream):
        """
        Sets the StreamHandler's stream to the specified value,
        if it is different.

        Returns the old stream, if the stream was changed, or None
        if it wasn't.
        """
        if stream is self.stream:
            result = None
        else:
            result = self.stream
            self.acquire()
            try:
                self.flush()
                self.stream = stream
            finally:
                self.release()
        return result

    def __repr__(self):
        level = logging.getLevelName(self.level)
        name = getattr(self.stream, 'name', '')
        #  bpo-36015: name can be an int
        name = str(name)
        if name:
            name += ' '
        return '<%s %s(%s)>' % (self.__class__.__name__, name, level)
