import tempfile
import logging
import picologging
import logging.handlers as logging_handlers
import picologging.handlers as picologging_handlers
import queue
import io


def filehandler_logging():
    with tempfile.NamedTemporaryFile() as f:
        logger = logging.Logger("test", logging.DEBUG)
        handler = logging.FileHandler(f.name)
        logger.handlers.append(handler)

        for _ in range(1_000):
            logger.debug("There has been a logging issue")
        handler.close()


def filehandler_picologging():
    with tempfile.NamedTemporaryFile() as f:
        logger = picologging.Logger("test", picologging.DEBUG)
        handler = picologging.FileHandler(f.name)
        logger.handlers.append(handler)

        for _ in range(1_000):
            logger.debug("There has been a picologging issue")
        handler.close()


def watchedfilehandler_logging():
    with tempfile.NamedTemporaryFile() as f:
        logger = logging.Logger("test", logging.DEBUG)
        handler = logging_handlers.WatchedFileHandler(f.name)
        logger.handlers.append(handler)

        for _ in range(1_000):
            logger.debug("There has been a logging issue")
        handler.close()


def watchedfilehandler_picologging():
    with tempfile.NamedTemporaryFile() as f:
        logger = picologging.Logger("test", picologging.DEBUG)
        handler = picologging_handlers.WatchedFileHandler(f.name)
        logger.handlers.append(handler)

        for _ in range(1_000):
            logger.debug("There has been a picologging issue")
        handler.close()


def rotatingfilehandler_logging():
    with tempfile.NamedTemporaryFile() as f:
        logger = logging.Logger("test", logging.DEBUG)
        handler = logging_handlers.RotatingFileHandler(
            f.name, maxBytes=10_000, backupCount=5
        )
        logger.handlers.append(handler)

        for _ in range(1_000):
            logger.debug("There has been a logging issue")
        handler.close()


def rotatingfilehandler_picologging():
    with tempfile.NamedTemporaryFile() as f:
        logger = picologging.Logger("test", picologging.DEBUG)
        handler = picologging_handlers.RotatingFileHandler(
            f.name, maxBytes=10_000, backupCount=5
        )
        logger.handlers.append(handler)

        for _ in range(1_000):
            logger.debug("There has been a picologging issue")
        handler.close()


def queuehandler_logging():
    logger = logging.Logger("test", picologging.DEBUG)
    q = queue.Queue()
    handler = logging_handlers.QueueHandler(q)
    logger.addHandler(handler)
    for _ in range(10_000):
        logger.debug("test")


def queuehandler_picologging():
    logger = picologging.Logger("test", picologging.DEBUG)
    q = queue.Queue()
    handler = picologging_handlers.QueueHandler(q)
    logger.addHandler(handler)
    for _ in range(10_000):
        logger.debug("test")


def queue_listener_logging():
    logger = logging.Logger("test", picologging.DEBUG)
    stream = io.StringIO()
    stream_handler = logging.StreamHandler(stream)
    q = queue.Queue()
    listener = logging_handlers.QueueListener(q, stream_handler)
    listener.start()
    handler = logging_handlers.QueueHandler(q)
    logger.addHandler(handler)
    for _ in range(1_000):
        logger.debug("test")

    listener.stop()


def queue_listener_picologging():
    logger = picologging.Logger("test", picologging.DEBUG)
    stream = io.StringIO()
    stream_handler = picologging.StreamHandler(stream)
    q = queue.Queue()
    listener = picologging_handlers.QueueListener(q, stream_handler)
    listener.start()
    handler = picologging_handlers.QueueHandler(q)
    logger.addHandler(handler)
    for _ in range(1_000):
        logger.debug("test")

    listener.stop()


def memoryhandler_logging():
    with tempfile.NamedTemporaryFile() as f:
        logger = logging.Logger("test", logging.DEBUG)
        target = logging.FileHandler(f.name)
        handler = logging_handlers.MemoryHandler(capacity=100, target=target)
        logger.handlers.append(handler)

        for _ in range(1_000):
            logger.debug("There has been a logging issue")
        handler.close()


def memoryhandler_picologging():
    with tempfile.NamedTemporaryFile() as f:
        logger = picologging.Logger("test", picologging.DEBUG)
        target = picologging.FileHandler(f.name)
        handler = picologging_handlers.MemoryHandler(capacity=100, target=target)
        logger.handlers.append(handler)

        for _ in range(1_000):
            logger.debug("There has been a picologging issue")
        handler.close()


__benchmarks__ = [
    (filehandler_logging, filehandler_picologging, "FileHandler()"),
    (
        watchedfilehandler_logging,
        watchedfilehandler_picologging,
        "WatchedFileHandler()",
    ),
    (
        rotatingfilehandler_logging,
        rotatingfilehandler_picologging,
        "RotatingFileHandler()",
    ),
    (queuehandler_logging, queuehandler_picologging, "QueueHandler()"),
    (
        queue_listener_logging,
        queue_listener_picologging,
        "QueueListener() + QueueHandler()",
    ),
    (memoryhandler_logging, memoryhandler_picologging, "MemoryHandler()"),
]
