import io
import threading

import pytest
from utils import filter_gc

import picologging
from picologging import Logger, StreamHandler


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_threaded_execution():
    logger = Logger("test", picologging.DEBUG)
    tmp = io.StringIO()
    handler = StreamHandler(tmp)
    logger.addHandler(handler)

    def _log_message():
        logger.debug("from thread")

    t = threading.Thread(target=_log_message)
    t.start()
    t.join()
    result = tmp.getvalue()
    assert result == "from thread\n"
