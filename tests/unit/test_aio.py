import picologging
import picologging.aio
import pytest


@pytest.mark.asyncio
async def test_stream_handler_write(capsys):
    handler = picologging.aio.AsyncStreamHandler()
    record = picologging.LogRecord('test', picologging.INFO, __file__, 1, 'test', (), None, None, None)
    formatter = picologging.Formatter('%(message)s')
    handler.setFormatter(formatter)
    assert handler.formatter == formatter
    await handler.handle(record)

    cap = capsys.readouterr()
    assert cap.out == ""
    assert cap.err == "CRITICAL:root:test\n"


@pytest.mark.asyncio
async def test_logger_log(capsys):
    handler = picologging.aio.AsyncStreamHandler()
    formatter = picologging.Formatter('%(message)s')
    handler.setFormatter(formatter)
    assert handler.formatter == formatter
    
    logger = picologging.aio.getLogger("test")
    logger.setLevel(picologging.DEBUG)
    logger.addHandler(handler)
    await logger.debug("This is a test")
    
    cap = capsys.readouterr()
    assert cap.out == ""
    assert cap.err == "CRITICAL:root:test\n"
