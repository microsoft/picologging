import os

import picologging


class WatchedFileHandler(picologging.FileHandler):
    """
    A handler for logging to a file, which watches the file
    to see if it has changed while in use. This can happen because of
    usage of programs such as newsyslog and logrotate which perform
    log file rotation. This handler, intended for use under Unix,
    watches the file to see if it has changed since the last emit.
    (A file has changed if its device or inode have changed.)
    If it has changed, the old file stream is closed, and the file
    opened to get a new stream.

    This handler is not appropriate for use under Windows, because
    under Windows open files cannot be moved or renamed - logging
    opens the files with exclusive locks - and so there is no need
    for such a handler. Furthermore, ST_INO is not supported under
    Windows; stat always returns zero for this value.
    """
    def __init__(self, filename, mode="a", encoding=None, delay=False):
        picologging.FileHandler.__init__(self, filename, mode=mode,
                                        encoding=encoding, delay=delay)
        self.dev, self.ino = -1, -1
        self._statstream()

    def _statstream(self):
        if self.stream:
            sres = os.fstat(self.stream.fileno())
            self.dev, self.ino = sres.st_dev, sres.st_ino

    def reopenIfNeeded(self):
        """
        Reopen log file if needed.

        Checks if the underlying file has changed, and if it
        has, close the old stream and reopen the file to get the
        current stream.
        """
        try:
            sres = os.stat(self.baseFilename)
        except FileNotFoundError:
            sres = None

        if not sres or sres.st_dev != self.dev or sres.st_ino != self.ino:
            if self.stream is not None:
                self.stream.flush()
                self.stream.close()
                self.stream = None  # See Issue #21742: _open () might fail.
                self.stream = self._open()
                self._statstream()

    def emit(self, record):
        """
        Emit a record.

        If underlying file has changed, reopen the file before emitting the
        record to it.
        """
        self.reopenIfNeeded()
        picologging.FileHandler.emit(self, record)


class BaseRotatingHandler(picologging.FileHandler):
    """
    Base class for handlers that rotate log files at a certain point.
    Not meant to be instantiated directly.  Instead, use RotatingFileHandler
    or TimedRotatingFileHandler.
    """
    namer = None
    rotator = None

    def __init__(self, filename, mode, encoding=None, delay=False):
        """
        Use the specified filename for streamed logging
        """
        picologging.FileHandler.__init__(self, filename, mode=mode,
                                        encoding=encoding, delay=delay)
        self.mode = mode
        self.encoding = encoding

    def shouldRollover(self, record):
        """
        Determine if rollover should occur.
        Should be implemented in inherited classes.
        """
        raise NotImplementedError()

    def doRollover(self, record):
        """
        Do a rollover.
        Should be implemented in inherited classes.
        """
        raise NotImplementedError()

    def emit(self, record):
        """
        Emit a record.
        Output the record to the file, catering for rollover as described
        in doRollover().
        """
        try:
            if self.shouldRollover(record):
                self.doRollover()
            picologging.FileHandler.emit(self, record)
        except Exception:
            self.handleError(record)

    def rotation_filename(self, default_name):
        """
        Modify the filename of a log file when rotating.
        This is provided so that a custom filename can be provided.
        The default implementation calls the 'namer' attribute of the
        handler, if it's callable, passing the default name to
        it. If the attribute isn't callable (the default is None), the name
        is returned unchanged.
        :param default_name: The default name for the log file.
        """
        if not callable(self.namer):
            result = default_name
        else:
            result = self.namer(default_name)
        return result

    def rotate(self, source, dest):
        """
        When rotating, rotate the current log.
        The default implementation calls the 'rotator' attribute of the
        handler, if it's callable, passing the source and dest arguments to
        it. If the attribute isn't callable (the default is None), the source
        is simply renamed to the destination.
        :param source: The source filename. This is normally the base
                       filename, e.g. 'test.log'
        :param dest:   The destination filename. This is normally
                       what the source is rotated to, e.g. 'test.log.1'.
        """
        if not callable(self.rotator):
            # Issue: https://bugs.python.org/issue18940
            # A file may not have been created if delay is True.
            if os.path.exists(source):
                os.rename(source, dest)
        else:
            self.rotator(source, dest)


class RotatingFileHandler(BaseRotatingHandler):
    """
    Handler for logging to a set of files, which switches from one file
    to the next when the current file reaches a certain size.
    """
    def __init__(self, filename, mode="a", maxBytes=0, backupCount=0,
                 encoding=None, delay=False):
        """
        Open the specified file and use it as the stream for logging.
        By default, the file grows indefinitely. You can specify particular
        values of maxBytes and backupCount to allow the file to rollover at
        a predetermined size.
        Rollover occurs whenever the current log file is nearly maxBytes in
        length. If backupCount is >= 1, the system will successively create
        new files with the same pathname as the base file, but with extensions
        ".1", ".2" etc. appended to it. For example, with a backupCount of 5
        and a base file name of "app.log", you would get "app.log",
        "app.log.1", "app.log.2", ... through to "app.log.5". The file being
        written to is always "app.log" - when it gets filled up, it is closed
        and renamed to "app.log.1", and if files "app.log.1", "app.log.2" etc.
        exist, then they are renamed to "app.log.2", "app.log.3" etc.
        respectively.
        If maxBytes is zero, rollover never occurs.
        """
        # If rotation/rollover is wanted, it doesn't make sense to use another
        # mode. If for example 'w' were specified, then if there were multiple
        # runs of the calling application, the logs from previous runs would be
        # lost if the 'w' is respected, because the log file would be truncated
        # on each run.
        if maxBytes > 0:
            mode = "a"
        BaseRotatingHandler.__init__(self, filename, mode, encoding=encoding, delay=delay)
        self.maxBytes = maxBytes
        self.backupCount = backupCount

    def doRollover(self):
        """
        Do a rollover, as described in __init__().
        """
        if self.stream:
            self.stream.close()
            self.stream = None
        if self.backupCount > 0:
            for i in range(self.backupCount - 1, 0, -1):
                sfn = self.rotation_filename("%s.%d" % (self.baseFilename, i))
                dfn = self.rotation_filename("%s.%d" % (self.baseFilename, i + 1))
                if os.path.exists(sfn):
                    if os.path.exists(dfn):
                        os.remove(dfn)
                    os.rename(sfn, dfn)
            dfn = self.rotation_filename(self.baseFilename + ".1")
            if os.path.exists(dfn):
                os.remove(dfn)
            self.rotate(self.baseFilename, dfn)
        if not self.delay:
            self.stream = self._open()

    def shouldRollover(self, record):
        """
        Determine if rollover should occur.
        Basically, see if the supplied record would cause the file to exceed
        the size limit we have.
        """
        # See bpo-45401: Never rollover anything other than regular files
        if os.path.exists(self.baseFilename) and not os.path.isfile(self.baseFilename):
            return False
        if self.stream is None:
            self.stream = self._open()
        if self.maxBytes > 0:
            msg = "%s\n" % self.format(record)
            self.stream.seek(0, 2)  # Due to non-posix-compliant Windows feature
            if self.stream.tell() + len(msg) >= self.maxBytes:
                return True
        return False
