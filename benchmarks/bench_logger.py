from io import StringIO
import logging
import picologging

logging.basicConfig()


def record_factory_logging():
    for _ in range(10_000):
        logging.LogRecord('hello', 40, '/serv/', 123, 'bork bork bork', (), None)


def record_factory_picologging():
    for _ in range(10_000):
        picologging.LogRecord('hello', 40, '/serv/', 123, 'bork bork bork', (), None)


def format_record_logging():
    f = logging.Formatter()
    record = logging.LogRecord('hello', 40, '/serv/', 123, 'bork bork bork', (), None)
    for _ in range(10_000):
        f.format(record)


def format_record_picologging():
    f = picologging.Formatter()
    record = picologging.LogRecord('hello', 40, '/serv/', 123, 'bork bork bork', (), None)
    for _ in range(10_000):
        f.format(record)


def logger_makerecord_logging():
    logger = logging.getLogger(__name__)
    for _ in range(10_000):
        logger.makeRecord('hello', 40, '/serv/', 123, 'bork bork bork', (), None)


def logger_makerecord_picologging():
    logger = logging.getLogger(__name__)
    picologging.install()
    for _ in range(10_000):
        r = logger.makeRecord('hello', 40, '/serv/', 123, 'bork bork bork', (), None)
    assert isinstance(r, picologging.LogRecord)
    picologging.uninstall()


def log_error_logging():
    logger = logging.getLogger()
    logger.handlers = []
    tmp = StringIO()

    handler = logging.StreamHandler(tmp)
    handler.setLevel(logging.DEBUG)
    formatter = logging.Formatter('%(name)s - %(levelname)s - %(message)s')
    handler.setFormatter(formatter)
    logger.addHandler(handler)

    for _ in range(10_000):
        logger.error("There has been a logging issue %s %s %s", 1, 2, 3)


def log_error_picologging():
    picologging.install()
    logger = logging.getLogger()
    logger.handlers = []
    tmp = StringIO()

    handler = logging.StreamHandler(tmp)
    handler.setLevel(logging.DEBUG)
    formatter = picologging.Formatter('%(name)s - %(levelname)s - %(message)s')
    handler.setFormatter(formatter)
    logger.addHandler(handler)

    for _ in range(10_000):
        logger.error("There has been a picologging issue %s %s %s", 1, 2, 3)
    picologging.uninstall()


__benchmarks__ = [
    (record_factory_logging, record_factory_picologging, "LogRecordFactory()"),
    (format_record_logging, format_record_picologging, "Formatter().format()"),
    (log_error_logging, log_error_picologging, "logging.error()"),
    (logger_makerecord_logging, logger_makerecord_picologging, "Logger.makeRecord()")
]
