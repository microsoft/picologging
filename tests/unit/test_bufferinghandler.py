import pytest
from utils import filter_gc

import picologging
from picologging.handlers import BufferingHandler, MemoryHandler


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_buffering_handler():
    logger = picologging.Logger("test", picologging.DEBUG)
    handler = BufferingHandler(capacity=1)
    logger.addHandler(handler)

    logger.debug("test")
    handler.close()

    assert handler.buffer == []


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_memory_handler(tmp_path):
    log_file = tmp_path / "log.txt"
    target = picologging.FileHandler(log_file)
    logger = picologging.Logger("test", picologging.DEBUG)
    handler = MemoryHandler(capacity=1, target=target)
    logger.addHandler(handler)

    logger.debug("test")
    handler.close()

    with open(log_file) as f:
        assert f.read() == "test\n"
    assert handler.buffer == []


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_memory_handler_set_target(tmp_path):
    log_file = tmp_path / "log.txt"
    target = picologging.FileHandler(log_file)
    logger = picologging.Logger("test", picologging.DEBUG)
    handler = MemoryHandler(capacity=1)
    handler.setTarget(target)
    logger.addHandler(handler)

    logger.debug("test")
    handler.close()

    with open(log_file) as f:
        assert f.read() == "test\n"
    assert handler.buffer == []
