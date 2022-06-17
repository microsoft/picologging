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


def format_record_with_date_logging():
    f = logging.Formatter("%(asctime)s - %(name)s - %(levelname)s - %(message)s")
    record = logging.LogRecord('hello', 40, '/serv/', 123, 'bork bork bork', (), None)
    for _ in range(10_000):
        f.format(record)


def format_record_with_date_picologging():
    f = picologging.Formatter("%(asctime)s - %(name)s - %(levelname)s - %(message)s")
    record = picologging.LogRecord('hello', 40, '/serv/', 123, 'bork bork bork', (), None)
    for _ in range(10_000):
        f.format(record)

def log_debug_logging(level=logging.DEBUG):
    logger = logging.Logger("test", level)
    tmp = StringIO()

    handler = logging.StreamHandler(tmp)
    handler.setLevel(level)
    formatter = logging.Formatter('%(name)s - %(levelname)s - %(message)s')
    handler.setFormatter(formatter)
    logger.handlers.append(handler)

    for _ in range(10_000):
        logger.debug("There has been a logging issue %s %s %s", 1, 2, 3)


def log_debug_picologging(level=logging.DEBUG):
    logger = picologging.Logger("test", level)
    tmp = StringIO()

    handler = logging.StreamHandler(tmp)
    handler.setLevel(level)
    formatter = picologging.Formatter('%(name)s - %(levelname)s - %(message)s')
    handler.setFormatter(formatter)
    logger.handlers.append(handler)

    for _ in range(10_000):
        logger.debug("There has been a picologging issue %s %s %s", 1, 2, 3)
    

def log_debug_outofscope_logging():
    log_debug_logging(logging.INFO)

def log_debug_outofscope_picologging():
    log_debug_picologging(logging.INFO)

__benchmarks__ = [
    (record_factory_logging, record_factory_picologging, "LogRecordFactory()"),
    (format_record_logging, format_record_picologging, "Formatter().format()"),
    (format_record_with_date_logging, format_record_with_date_picologging, "Formatter().format() with date"),
    (log_debug_logging, log_debug_picologging, "Logger(level=DEBUG).debug()"),
    (log_debug_outofscope_logging, log_debug_outofscope_picologging, "Logger(level=INFO).debug()"),
]
