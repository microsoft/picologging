import io
import queue

import picologging
from picologging.handlers import QueueHandler, QueueListener


def test_queue_handler_dispatch():
    logger = picologging.Logger("test", picologging.DEBUG)
    q = queue.Queue()
    handler = QueueHandler(q)
    logger.addHandler(handler)
    logger.debug("test")
    record = q.get(block=False)
    assert record
    assert record.levelno == picologging.DEBUG
    assert record.name == "test"
    assert record.msg == "test"
    assert record.args is None
    assert record.exc_info is None


def test_queue_listener():
    logger = picologging.Logger("test", picologging.DEBUG)
    stream = io.StringIO()
    stream_handler = picologging.StreamHandler(stream)
    q = queue.Queue()
    listener = QueueListener(q, stream_handler)
    listener.start()
    handler = QueueHandler(q)
    logger.addHandler(handler)
    logger.debug("test")

    listener.stop()
    assert stream.getvalue() == "test\n"


def test_queue_handler_handle_exception():
    logger = picologging.Logger("test", picologging.DEBUG)
    q = queue.Queue(maxsize=1)
    handler = QueueHandler(q)
    logger.addHandler(handler)

    handler.queue = None
    logger.debug("test")


def test_queue_handler_format():
    logger = picologging.getLogger("picologging_test")
    logger.setLevel(picologging.INFO)
    stream = io.StringIO()
    stream_handler = picologging.StreamHandler(stream)
    q = queue.Queue()
    listener = QueueListener(q, stream_handler)
    listener.start()
    handler = QueueHandler(q)
    handler.setLevel(picologging.DEBUG)
    handler.setFormatter(
        picologging.Formatter("%(levelname)s - %(name)s - %(message)s")
    )
    logger.addHandler(handler)
    logger.info("Testing now!")

    listener.stop()
    assert stream.getvalue() == "INFO - picologging_test - Testing now!\n"
