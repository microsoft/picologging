import atexit
from io import StringIO
from logging import StreamHandler as STDLibStreamHandler, getLogger as get_stdlib_logger, config as stdlib_config
from logging.handlers import QueueHandler as STDLibQueueHandler, QueueListener as STDLibQueueListener
from queue import Queue
from typing import Any, List, Optional

import pytest as pytest
from picologging import getLogger as get_picologging_logger, config as picologging_config
from picologging._picologging import StreamHandler as PicologgingStreamHandler
from picologging.handlers import QueueHandler as PicologgingQueueHandler, QueueListener as PicologgingQueueListener

def resolve_handlers(handlers: List[Any]) -> List[Any]:
    """Converts list of string of handlers to the object of respective handler.
    Indexing the list performs the evaluation of the object.
    Args:
        handlers: An instance of 'ConvertingList'
    Returns:
        A list of resolved handlers.
    Notes:
        Due to missing typing in 'typeshed' we cannot type this as ConvertingList for now.
    """
    return [handlers[i] for i in range(len(handlers))]



class STDLibQueueListenerHandler(STDLibQueueHandler):
    def __init__(self, handlers: Optional[List[Any]] = None) -> None:
        """Configures queue listener and handler to support non-blocking
        logging configuration.
        Args:
            handlers: Optional 'CovertingList'
        """
        super().__init__(Queue(-1))
        if handlers:
            handlers = resolve_handlers(handlers)
        else:
            handlers = [STDLibStreamHandler(StringIO())]
        self.listener = STDLibQueueListener(self.queue, *handlers)
        self.listener.start()

        atexit.register(self.listener.stop)


class PicologgingQueueListenerHandler(PicologgingQueueHandler):
    def __init__(self, handlers: Optional[List[Any]] = None) -> None:
        """Configures queue listener and handler to support non-blocking
        logging configuration.
        Args:
            handlers: Optional 'CovertingList'
        Notes:
            - Requires `picologging` to be installed.
        """
        super().__init__(Queue(-1))
        if handlers:
            handlers = resolve_handlers(handlers)
        else:
            handlers = [PicologgingStreamHandler(StringIO())]
        self.listener = PicologgingQueueListener(self.queue, *handlers)
        self.listener.start()

        atexit.register(self.listener.stop)


stdlib_config.dictConfig({
    "version": 1,
    "handlers": {
        "console": {
            "class": "logging.StreamHandler",
            "level": "DEBUG",
            "formatter": "standard",
        },
        "stdlib_queue_listener": {
            "class": "tests.unit.STDLibQueueListenerHandler",
            "level": "DEBUG",
            "formatter": "standard",
        },
    },
    "disable_existing_loggers": False,
    "formatters": {
        "standard": {"format": "%(levelname)s - %(asctime)s - %(name)s - %(module)s - %(message)s"}
    },
    "loggers": {
        "stdlib_test": {
            "level": "INFO",
            "handlers": ["stdlib_queue_listener"],
        },
    }
})

stdlib_logger = get_stdlib_logger("stdlib_test")


def test_stdlib_logger(caplog: Any) -> None:
    with caplog.at_level("INFO"):
        stdlib_logger.info("Testing now!")
        assert "Testing now!" in caplog.text


picologging_config.dictConfig({
    "version": 1,
    "handlers": {
        "console": {
            "class": "picologging.StreamHandler",
            "level": "DEBUG",
            "formatter": "standard",
        },
        "picologging_queue_listener": {
            "class": "tests.unit.PicologgingQueueListenerHandler",
            "level": "DEBUG",
            "formatter": "standard",
        },
    },
    "disable_existing_loggers": False,
    "formatters": {
        "standard": {"format": "%(levelname)s - %(asctime)s - %(name)s - %(module)s - %(message)s"}
    },
    "loggers": {
        "picologging_test": {
            "level": "INFO",
            "handlers": ["picologging_queue_listener"],
        },
    }
})

picologging_logger = get_picologging_logger("picologging_test")


@pytest.mark.xfail(reason="https://github.com/microsoft/picologging/issues/90")
def test_picologger_logger(caplog: Any) -> None:
    with caplog.at_level("INFO"):
        picologging_logger.info("Testing now!")
        assert "Testing now!" in caplog.text
