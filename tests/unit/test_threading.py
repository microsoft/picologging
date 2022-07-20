import threading
import picologging
from picologging import Logger, StreamHandler
import io


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
