import atexit
from io import StringIO
import logging
from logging import StreamHandler as STDLibStreamHandler, getLogger as get_stdlib_logger, config as stdlib_config
from logging.handlers import QueueHandler as STDLibQueueHandler, QueueListener as STDLibQueueListener
from queue import Queue
from typing import Any, List, Optional

import pytest as pytest
import src.picologging
from src.picologging import getLogger as get_picologging_logger, config as picologging_config
from src.picologging._picologging import StreamHandler as PicologgingStreamHandler
from src.picologging.handlers import QueueHandler as PicologgingQueueHandler, QueueListener as PicologgingQueueListener

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
            print('Stream handler')
            handlers = [STDLibStreamHandler(StringIO())]
        print('Creating queue with handlers')
        self.listener = STDLibQueueListener(self.queue, *handlers)
        self.listener.start()
        print('Started listener')
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


def test_stdlib_logger(caplog: Any) -> None:
    stdlib_config.dictConfig({
        "version": 1,
        "handlers": {
            "console": {
                "class": "logging.StreamHandler",
                "level": "DEBUG",
                "formatter": "standard",
            },
            "stdlib_queue_listener": {
                "class": __name__ + ".STDLibQueueListenerHandler",
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
    logger_obj = get_stdlib_logger()
    orig_level = logger_obj.level
    logger_obj.setLevel(logging.INFO)
    stdlib_logger.info("Testing now!")
    logger_obj.setLevel(orig_level)


#@pytest.mark.xfail(reason="https://github.com/microsoft/picologging/issues/90")
def test_picologger_logger(caplog: Any) -> None:
    picologging_config.dictConfig({
        "version": 1,
        "handlers": {
            "console": {
                "class": "src.picologging.StreamHandler",
                "level": "DEBUG",
                "formatter": "standard",
            },
            "picologging_queue_listener": {
                "class": __name__ + ".PicologgingQueueListenerHandler",
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
    logger_obj = get_picologging_logger()
    orig_level = logger_obj.level
    logger_obj.setLevel(src.picologging.INFO)
    picologging_logger.info("Testing now!")
    logger_obj.setLevel(orig_level)