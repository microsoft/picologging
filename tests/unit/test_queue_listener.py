import io
import queue
from typing import Any

import pytest

from picologging import INFO, Logger, StreamHandler, handlers

logger = Logger("test", INFO)
stream = io.StringIO()
stream_handler = StreamHandler(stream)
q = queue.Queue()
listener = handlers.QueueListener(q, stream_handler)
listener.start()
handler = handlers.QueueHandler(q)
logger.addHandler(handler)


@pytest.mark.xfail(reason="https://github.com/microsoft/picologging/issues/90")
def test_queue_listener_handler(caplog: Any) -> None:
    with caplog.at_level("INFO"):
        logger.info("Testing now!")
        assert "Testing now!" in caplog.text
