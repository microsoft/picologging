import os

import picologging

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
            # Issue 18940: A file may not have been created if delay is True.
            if os.path.exists(source):
                os.rename(source, dest)
        else:
            self.rotator(source, dest)
