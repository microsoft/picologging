import atexit
from io import StringIO
from logging import StreamHandler as STDLibStreamHandler
from logging.handlers import QueueHandler as STDLibQueueHandler, QueueListener as STDLibQueueListener
from queue import Queue
from typing import Any, List, Optional

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
