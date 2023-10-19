"""
Create 100,000  logger instances and run a log test on them
"""

import queue
import io

import picologging
import picologging.handlers


def log():
    logger = picologging.Logger("test", picologging.DEBUG)
    stream = io.StringIO()
    stream_handler = picologging.StreamHandler(stream)
    q = queue.SimpleQueue()
    listener = picologging.handlers.QueueListener(q, stream_handler)
    listener.start()
    handler = picologging.handlers.QueueHandler(q)
    logger.addHandler(handler)
    for _ in range(1_000):
        logger.debug("test")

    listener.stop()
    assert stream.getvalue() != ""


if __name__ == "__main__":
    log()
