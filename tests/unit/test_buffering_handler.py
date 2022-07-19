import picologging
from picologging.handlers import BufferingHandler


def test_buffering_handler():
    logger = picologging.Logger("test", picologging.DEBUG)
    handler = BufferingHandler(capacity=1)
    logger.addHandler(handler)

    logger.debug("test")
    handler.close()

    assert handler.buffer == []
