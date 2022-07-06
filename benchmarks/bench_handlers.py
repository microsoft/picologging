import tempfile
import logging
import picologging
import logging.handlers as logging_handlers
import picologging.handlers as picologging_handlers


def filehandler_logging():
    with tempfile.NamedTemporaryFile() as f:
        logger = logging.Logger("test", logging.DEBUG)
        handler = logging.FileHandler(f.name)
        logger.handlers.append(handler)

        for _ in range(1_000):
            logger.debug("There has been a logging issue")


def filehandler_picologging():
    with tempfile.NamedTemporaryFile() as f:
        logger = picologging.Logger("test", picologging.DEBUG)
        handler = picologging.FileHandler(f.name)
        logger.handlers.append(handler)

        for _ in range(1_000):
            logger.debug("There has been a picologging issue")


def watchedfilehandler_logging():
    with tempfile.NamedTemporaryFile() as f:
        logger = logging.Logger("test", logging.DEBUG)
        handler = logging_handlers.WatchedFileHandler(f.name)
        logger.handlers.append(handler)

        for _ in range(1_000):
            logger.debug("There has been a logging issue")


def watchedfilehandler_picologging():
    with tempfile.NamedTemporaryFile() as f:
        logger = picologging.Logger("test", picologging.DEBUG)
        handler = picologging_handlers.WatchedFileHandler(f.name)
        logger.handlers.append(handler)

        for _ in range(1_000):
            logger.debug("There has been a picologging issue")


def rotatingfilehandler_logging():
    with tempfile.NamedTemporaryFile() as f:
        logger = logging.Logger("test", logging.DEBUG)
        handler = logging_handlers.RotatingFileHandler(f.name, maxBytes=10_000, backupCount=5)
        logger.handlers.append(handler)

        for _ in range(1_000):
            logger.debug("There has been a logging issue")


def rotatingfilehandler_picologging():
    with tempfile.NamedTemporaryFile() as f:
        logger = picologging.Logger("test", picologging.DEBUG)
        handler = picologging_handlers.RotatingFileHandler(f.name, maxBytes=10_000, backupCount=5)
        logger.handlers.append(handler)

        for _ in range(1_000):
            logger.debug("There has been a picologging issue")


__benchmarks__ = [
    (filehandler_logging, filehandler_picologging, "FileHandler()"),
    (watchedfilehandler_logging, watchedfilehandler_picologging, "WatchedFileHandler()"),
    (rotatingfilehandler_logging, rotatingfilehandler_picologging, "RotatingFileHandler()"),
]
