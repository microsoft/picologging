from picologging.handlers import QueueHandler
import picologging
import queue


def test_queue_handler_dispatch():
    logger = picologging.Logger("test", picologging.DEBUG)
    q = queue.Queue()
    handler = QueueHandler(q)
    logger.addHandler(handler)
    logger.debug("test")
    assert q.get(block=False) == "test"
