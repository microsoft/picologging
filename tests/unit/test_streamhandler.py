import io
import logging
import sys

import pytest
from utils import filter_gc

import picologging


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_stream_handler():
    stream = io.StringIO()
    handler = picologging.StreamHandler(stream)
    record = picologging.LogRecord(
        "test", picologging.INFO, __file__, 1, "test", (), None, None, None
    )
    formatter = picologging.Formatter("%(message)s")
    handler.setFormatter(formatter)
    assert handler.formatter == formatter
    handler.handle(record)
    assert stream.getvalue() == "test\n"


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_stream_handler_defaults_to_stderr():
    handler = picologging.StreamHandler()
    assert handler.stream == sys.stderr

    handler = picologging.StreamHandler(None)
    assert handler.stream == sys.stderr


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_stream_handler_bad_init_args():
    with pytest.raises(TypeError):
        picologging.StreamHandler(1, 2, 3, 4)

    with pytest.raises(TypeError):
        picologging.StreamHandler(dog=1)


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_non_flushable_stream():
    class TestStream:
        def write(self, data):
            pass

    handler = picologging.StreamHandler(TestStream())
    assert not handler.flush()


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_emit_no_args():
    handler = picologging.StreamHandler()
    with pytest.raises(ValueError):
        handler.emit()


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_emit_invalid_args_type():
    handler = picologging.StreamHandler()
    with pytest.raises(TypeError):
        handler.emit(1234)


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_stream_write_raises_error():
    class TestStream:
        def write(self, data):
            raise Exception("blerg")

    handler = picologging.StreamHandler(TestStream())
    with pytest.raises(Exception):
        handler.emit("foo 123")


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_set_stream():
    class TestStream:
        def write(self, data):
            pass

        def flush(self):
            pass

    a = TestStream()
    handler = picologging.StreamHandler(a)
    assert handler.stream is a
    b = TestStream()
    handler.setStream(b)
    assert handler.stream is b


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_set_stream_return_value():
    h = picologging.StreamHandler()
    stream = io.StringIO()
    old = h.setStream(stream)
    assert old is sys.stderr
    actual = h.setStream(old)
    assert actual is stream
    # test that setting to existing value returns None
    actual = h.setStream(old)
    assert actual is None


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_streamhandler_repr():
    class StreamWithName:
        level = picologging.NOTSET
        name = "beyonce"

    handler = picologging.StreamHandler(StreamWithName())
    assert repr(handler) == "<StreamHandler beyonce (NOTSET)>"

    class StreamWithIntName:
        level = picologging.NOTSET
        name = 2

    handler = picologging.StreamHandler(StreamWithIntName())
    assert repr(handler) == "<StreamHandler 2 (NOTSET)>"


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_streamhandler_handle_return_value():
    handler = picologging.StreamHandler()
    record = picologging.LogRecord(
        "test", picologging.WARNING, __file__, 1, "test", (), None, None, None
    )

    assert handler.handle(record) is True
    assert handler.emit(record) is None

    class TestFilter(picologging.Filter):
        def filter(self, record):
            return False

    handler.addFilter(TestFilter())
    assert handler.handle(record) is None
    assert handler.emit(record) is None


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_emit_subclass():
    class DerivedLogRecord(picologging.LogRecord):
        pass

    record = DerivedLogRecord(
        "hello", logging.WARNING, __file__, 123, "bork boom", (), None
    )

    assert DerivedLogRecord.__base__ is picologging.LogRecord
    assert record.message is None
    assert record.getMessage() == "bork boom"
    assert record.message == "bork boom"
    assert record.message == "bork boom"
    stream = io.StringIO()
    handler = picologging.StreamHandler(stream)
    handler.emit(record)
    assert stream.getvalue() == "bork boom\n"
