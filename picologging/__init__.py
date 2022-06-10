import logging

from ._picologging import LogRecord, PercentStyle, Formatter

def install():
    """ Install the picologging record and logger """
    if len(logging.root.handlers) == 0:
        raise ValueError("There are no handlers configured. Make sure logging is setup first (logging.basicSetup())")

    logging.setLogRecordFactory(LogRecord)
    logging._STYLES['%'] = (PercentStyle, logging.BASIC_FORMAT)
    logging.Logger.manager.logRecordFactory = LogRecord
    for h in logging.Logger.root.handlers:
        h.setFormatter(Formatter(h.formatter._fmt))
    logging._defaultFormatter = Formatter()

def uninstall():
    """ Uninstall the picologging record and logger """
    logging.setLogRecordFactory(logging.LogRecord)
    logging.Logger.manager.logRecordFactory = logging.LogRecord
    logging._STYLES['%'] = (logging.PercentStyle, logging.BASIC_FORMAT)
    for h in logging.Logger.root.handlers:
        h.setFormatter(logging.Formatter(h.formatter._fmt))
    logging._defaultFormatter = logging.Formatter()

