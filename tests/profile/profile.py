import queue
from io import StringIO

import picologging
import picologging.handlers


def run_profile(level=picologging.DEBUG):
    logger = picologging.Logger("test", level)
    tmp = StringIO()
    queue_ = queue.SimpleQueue()
    handler = picologging.handlers.QueueHandler(queue_)
    handler.setLevel(level)
    formatter = picologging.Formatter("%(name)s - %(levelname)s - %(message)s")
    handler.setFormatter(formatter)
    logger.handlers.append(handler)

    for _ in range(1_000_000):
        logger.debug("There has been a picologging issue %s %s %s", 1, 2, 3)

    assert tmp.getvalue() != ""


run_profile()
