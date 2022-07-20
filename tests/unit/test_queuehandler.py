from picologging.handlers import QueueHandler, QueueListener
import picologging
import queue
import io


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
    assert record.args == ()
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
