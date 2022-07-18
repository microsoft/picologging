import asyncio
from typing import IO, Optional, List
from picologging import DEBUG, INFO, WARNING, ERROR, CRITICAL, Filterer, _checkLevel, NOTSET, Handler, getLevelName, Formatter, LogRecord, Manager
import sys
import threading
import os
import sys
import traceback
import io
from logging import currentframe

_defaultFormatter: Formatter = Formatter()

class AsyncHandler(Filterer):
    """
    Handler instances dispatch logging events to specific destinations.

    The base handler class. Acts as a placeholder which defines the Handler
    interface. Handlers can optionally use Formatter instances to format
    records as desired. By default, no formatter is specified; in this case,
    the 'raw' message as determined by record.message is logged.
    """
    def __init__(self, level=NOTSET):
        """
        Initializes the instance - basically setting the formatter to None
        and the filter list to empty.
        """
        super().__init__()
        self._name = None
        self.level = _checkLevel(level)
        self.formatter = None
        self._closed = False
        # Add the handler to the global _handlerList (for cleanup on shutdown)
        # _addHandlerRef(self)
        self.createLock()

    def createLock(self):
        """
        Acquire a thread lock for serializing access to the underlying I/O.
        """
        self.lock = threading.RLock()

    def acquire(self):
        """
        Acquire the I/O thread lock.
        """
        if self.lock:
            self.lock.acquire()

    def release(self):
        """
        Release the I/O thread lock.
        """
        if self.lock:
            self.lock.release()

    def setLevel(self, level):
        """
        Set the logging level of this handler.  level must be an int or a str.
        """
        self.level = _checkLevel(level)

    def format(self, record):
        """
        Format the specified record.

        If a formatter is set, use it. Otherwise, use the default formatter
        for the module.
        """
        if self.formatter:
            fmt = self.formatter
        else:
            fmt = _defaultFormatter
        return fmt.format(record)

    async def emit(self, record):
        """
        Do whatever it takes to actually log the specified logging record.

        This version is intended to be implemented by subclasses and so
        raises a NotImplementedError.
        """
        raise NotImplementedError('emit must be implemented '
                                  'by Handler subclasses')

    async def handle(self, record):
        """
        Conditionally emit the specified logging record.

        Emission depends on filters which may have been added to the handler.
        Wrap the actual emission of the record with acquisition/release of
        the I/O thread lock. Returns whether the filter passed the record for
        emission.
        """
        rv = self.filter(record)
        if rv:
            self.acquire()
            try:
                await self.emit(record)
            finally:
                self.release()
        return rv

    def setFormatter(self, fmt):
        """
        Set the formatter for this handler.
        """
        self.formatter = fmt

    def flush(self):
        """
        Ensure all logging output has been flushed.

        This version does nothing and is intended to be implemented by
        subclasses.
        """
        pass

    def handleError(self, record):
        """
        Handle errors which occur during an emit() call.

        This method should be called from handlers when an exception is
        encountered during an emit() call. If raiseExceptions is false,
        exceptions get silently ignored. This is what is mostly wanted
        for a logging system - most users will not care about errors in
        the logging system, they are more interested in application errors.
        You could, however, replace this with a custom handler if you wish.
        The record which was being processed is passed in to this method.
        """
        if sys.stderr:  # see issue 13807
            t, v, tb = sys.exc_info()
            try:
                sys.stderr.write('--- Logging error ---\n')
                traceback.print_exception(t, v, tb, None, sys.stderr)
                sys.stderr.write('Call stack:\n')
                # Walk the stack frame up until we're out of logging,
                # so as to print the calling context.
                frame = tb.tb_frame
                while (frame and os.path.dirname(frame.f_code.co_filename) ==
                       __path__[0]):
                    frame = frame.f_back
                if frame:
                    traceback.print_stack(frame, file=sys.stderr)
                else:
                    # couldn't find the right stack frame, for some reason
                    sys.stderr.write('Logged from file %s, line %s\n' % (
                                     record.filename, record.lineno))
                # Issue 18671: output logging message and arguments
                try:
                    sys.stderr.write('Message: %r\n'
                                     'Arguments: %s\n' % (record.msg,
                                                          record.args))
                except RecursionError:  # See issue 36272
                    raise
                except Exception:
                    sys.stderr.write('Unable to print the message and arguments'
                                     ' - possible formatting error.\nUse the'
                                     ' traceback above to help find the error.\n'
                                    )
            except OSError: #pragma: no cover
                pass    # see issue 5971
            finally:
                del t, v, tb

    def __repr__(self):
        level = getLevelName(self.level)
        return '<%s (%s)>' % (self.__class__.__name__, level)

asyncLastResort: AsyncHandler = None # forward decl

class AsyncLogger(Filterer):
    def __init__(self, name: str, level: int=NOTSET):
        """
        Initialize the logger with a name and an optional level.
        """
        Filterer.__init__(self)
        self.name: str = name
        self.level: int = _checkLevel(level)
        self.parent: Optional[AsyncLogger] = None
        self.propagate: bool = True
        self.handlers: List[AsyncHandler] = []
        self.disabled: bool = False
        self._cache = {}

    def addHandler(self, handler: AsyncHandler):
        if handler not in self.handlers:
            self.handlers.append(handler)

    async def callHandlers(self, record):
        """
        Pass a record to all relevant handlers.

        Loop through all handlers for this logger and its parents in the
        logger hierarchy. If no handler was found, output a one-off error
        message to sys.stderr. Stop searching up the hierarchy whenever a
        logger with the "propagate" attribute set to zero is found - that
        will be the last logger whose handlers are called.
        """
        c = self
        found: int = 0
        while c:
            for hdlr in c.handlers:
                found = found + 1
                if record.levelno >= hdlr.level:
                    await hdlr.handle(record)
            if not c.propagate:
                c = None    #break out
            else:
                c = c.parent
        if (found == 0):
            if record.levelno >= asyncLastResort.level:
                await asyncLastResort.handle(record)

    async def handle(self, record):
        """
        Call the handlers for the specified record.

        This method is used for unpickled records received from a socket, as
        well as those created locally. Logger-level filtering is applied.
        """
        if (not self.disabled) and self.filter(record):
            await self.callHandlers(record)
    
    def getEffectiveLevel(self):
        """
        Get the effective level for this logger.

        Loop through this logger and its parents in the logger hierarchy,
        looking for a non-zero logging level. Return the first one found.
        """
        logger = self
        while logger:
            if logger.level:
                return logger.level
            logger = logger.parent
        return NOTSET

    def isEnabledFor(self, level):
        """
        Is this logger enabled for level 'level'?
        """
        if self.disabled:
            return False

        try:
            return self._cache[level]
        except KeyError:
            if self.manager.disable >= level:
                is_enabled = self._cache[level] = False
            else:
                is_enabled = self._cache[level] = (
                    level >= self.getEffectiveLevel()
                )

            return is_enabled

    async def debug(self, msg, *args, **kwargs):
        if self.isEnabledFor(DEBUG):
            await self._log(DEBUG, msg, args, **kwargs)

    async def info(self, msg, *args, **kwargs):
        if self.isEnabledFor(INFO):
            await self._log(INFO, msg, args, **kwargs)

    async def warning(self, msg, *args, **kwargs):
        if self.isEnabledFor(WARNING):
            await self._log(DEBUG, msg, args, **kwargs)

    async def error(self, msg, *args, **kwargs):
        if self.isEnabledFor(ERROR):
            await self._log(ERROR, msg, args, **kwargs)

    async def critical(self, msg, *args, **kwargs):
        if self.isEnabledFor(CRITICAL):
            await self._log(CRITICAL, msg, args, **kwargs)

    async def exception(self, msg, *args, exc_info=True, **kwargs):
        await self.error(msg, *args, exc_info=exc_info, **kwargs)

    async def _log(self, level, msg, args, exc_info=None, extra=None, stack_info=False,
             stacklevel=1):
        """
        Low-level logging routine which creates a LogRecord and then calls
        all the handlers of this logger to handle the record.
        """
        sinfo = None
        try:
            fn, lno, func, sinfo = self.findCaller(stack_info, stacklevel)
        except ValueError: # pragma: no cover
            fn, lno, func = "(unknown file)", 0, "(unknown function)"
    
        if exc_info:
            if isinstance(exc_info, BaseException):
                exc_info = (type(exc_info), exc_info, exc_info.__traceback__)
            elif not isinstance(exc_info, tuple):
                exc_info = sys.exc_info()
        # TODO: Add extras
        record = LogRecord(self.name, level, fn, lno, msg, args,
                                 exc_info, func, sinfo)
        await self.handle(record)

    async def log(self, level, msg, *args, **kwargs):
        """
        Log 'msg % args' with the integer severity 'level'.

        To pass exception information, use the keyword argument exc_info with
        a true value, e.g.

        logger.log(level, "We have a %s", "mysterious problem", exc_info=1)
        """
        if not isinstance(level, int):
            raise TypeError("level must be an integer")
        if self.isEnabledFor(level):
            await self._log(level, msg, args, **kwargs)

    def findCaller(self, stack_info=False, stacklevel=1):
        """
        Find the stack frame of the caller so that we can note the source
        file name, line number and function name.
        """
        f = currentframe()
        #On some versions of IronPython, currentframe() returns None if
        #IronPython isn't run with -X:Frames.
        if f is not None:
            f = f.f_back
        orig_f = f
        while f and stacklevel > 1:
            f = f.f_back
            stacklevel -= 1
        if not f:
            f = orig_f
        rv = "(unknown file)", 0, "(unknown function)", None
        while hasattr(f, "f_code"):
            co = f.f_code
            filename = os.path.normcase(co.co_filename)
            if filename == __file__:
                f = f.f_back
                continue
            sinfo = None
            if stack_info:
                sio = io.StringIO()
                sio.write('Stack (most recent call last):\n')
                traceback.print_stack(f, file=sio)
                sinfo = sio.getvalue()
                if sinfo[-1] == '\n':
                    sinfo = sinfo[:-1]
                sio.close()
            rv = (co.co_filename, f.f_lineno, co.co_name, sinfo)
            break
        return rv

    def setLevel(self, level):
        """
        Set the logging level of this logger.  level must be an int or a str.
        """
        self.level = _checkLevel(level)


class AsyncFileHandler(AsyncHandler):
    def __init__(self, filename:str , mode: str='a', encoding: Optional[str]=None, delay: bool=False):
        super().__init__(None)
        self.filename = filename
        self.mode = mode
        self.encoding = encoding
        self.delay = delay

    async def emit(self, record):
        pass

    async def close(self):
        pass

    async def flush(self):
        pass


async def _stream_as_async(stream: IO[str]):
    loop = asyncio.get_event_loop()
    reader = asyncio.StreamReader()
    w_transport, w_protocol = await loop.connect_write_pipe(asyncio.streams.FlowControlMixin, stream)
    return asyncio.StreamWriter(w_transport, w_protocol, reader, loop)


class AsyncStreamHandler(AsyncHandler):
    terminator = '\n'

    def __init__(self, stream: Optional[IO[str]]=None, level=NOTSET):
        """
        Initialize the handler.

        If stream is not specified, sys.stderr is used.
        """
        super().__init__(level)
        if stream is None:
            stream = sys.stderr
        self.stream: IO[str] = stream
        self._stream_writer = None

    async def emit(self, record):
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
            if not self._stream_writer:
                self._stream_writer = await _stream_as_async(self.stream)
            await self._stream_writer.write(msg + self.terminator)
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
        level = getLevelName(self.level)
        name = getattr(self.stream, 'name', '')
        #  bpo-36015: name can be an int
        name = str(name)
        if name:
            name += ' '
        return '<%s %s(%s)>' % (self.__class__.__name__, name, level)


asyncLastResort = AsyncStreamHandler(sys.stderr, WARNING)

root = AsyncLogger(name="root", level=WARNING)
manager = Manager(root, cls=AsyncLogger)

def getLogger(name=None):
    """
    Return a logger with the specified name, creating it if necessary.

    If no name is specified, return the root logger.
    """
    if not name or isinstance(name, str) and name == root.name:
        return root
    return manager.getLogger(name)