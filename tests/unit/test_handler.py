import pytest
from utils import filter_gc

import picologging


@pytest.mark.limit_leaks("64B", filter_fn=filter_gc)
def test_basic_handler():
    handler = picologging.Handler()
    record = picologging.LogRecord(
        "test", picologging.INFO, "test", 1, "test", (), None, None, None
    )
    with pytest.raises(NotImplementedError):
        handler.handle(record)
    with pytest.raises(NotImplementedError):
        handler.emit(None)


@pytest.mark.limit_leaks("128B", filter_fn=filter_gc)
def test_custom_handler():
    class CustomHandler(picologging.Handler):
        def __init__(self):
            super().__init__()
            self.records = []

        def emit(self, record):
            self.records.append(record)

    handler = CustomHandler()
    record = picologging.LogRecord(
        "test", picologging.INFO, __file__, 1, "test", (), None, None, None
    )
    formatter = picologging.Formatter("%(message)s")
    handler.setFormatter(formatter)
    handler.handle(record)
    assert len(handler.records) == 1
    assert handler.records[0] == record


@pytest.mark.limit_leaks("64B", filter_fn=filter_gc)
def test_delete_handler():
    handler = picologging.Handler()
    del handler


@pytest.mark.limit_leaks("64B", filter_fn=filter_gc)
def test_add_acquire_release():
    handler = picologging.Handler()
    handler.acquire()
    assert handler.release() is None


@pytest.mark.limit_leaks("64B", filter_fn=filter_gc)
def test_init_with_name():
    handler = picologging.Handler(name="test")
    assert handler.name == "test"


@pytest.mark.limit_leaks("64B", filter_fn=filter_gc)
def test_init_with_level():
    handler = picologging.Handler(level=picologging.DEBUG)
    assert handler.level == picologging.DEBUG


@pytest.mark.limit_leaks("64B", filter_fn=filter_gc)
def test_get_set_name():
    handler = picologging.Handler(name="test")
    assert handler.get_name() == "test"
    handler.set_name("foo")
    assert handler.name == "foo"
    assert handler.get_name() == "foo"


@pytest.mark.limit_leaks("64B", filter_fn=filter_gc)
def test_flush():
    handler = picologging.Handler()
    assert not handler.flush()


@pytest.mark.limit_leaks("64B", filter_fn=filter_gc)
def test_close():
    handler = picologging.Handler()
    assert not handler.close()


@pytest.mark.limit_leaks("64B", filter_fn=filter_gc)
def test_createLock():
    handler = picologging.Handler()
    assert not handler.createLock()


@pytest.mark.limit_leaks("64B", filter_fn=filter_gc)
def test_filtered_out():
    def filter_out(f):
        return False

    class CustomHandler(picologging.Handler):
        def __init__(self):
            super().__init__()
            self.records = []

        def emit(self, record):
            raise Exception("This should not be called")

    handler = CustomHandler()
    handler.addFilter(filter_out)
    record = picologging.LogRecord(
        "test", picologging.INFO, __file__, 1, "test", (), None, None, None
    )
    assert not handler.handle(record)


@pytest.mark.limit_leaks("64B", filter_fn=filter_gc)
def test_set_level_nonint():
    handler = picologging.Handler()
    with pytest.raises(TypeError):
        handler.setLevel("potato")


@pytest.mark.limit_leaks("64B", filter_fn=filter_gc)
def test_custom_formatter():
    class CustomFormatter:
        def format(self, record):
            return "foo"

    handler = picologging.Handler()
    handler.setFormatter(CustomFormatter())
    record = picologging.LogRecord(
        "test", picologging.INFO, __file__, 1, "test", (), None, None, None
    )
    assert handler.format(record) == "foo"


@pytest.mark.limit_leaks("64B", filter_fn=filter_gc)
def test_handle_error():
    handler = picologging.Handler()
    record = picologging.LogRecord(
        "test", picologging.INFO, __file__, 1, "test", (), None, None, None
    )
    assert not handler.handleError(record)


@pytest.mark.limit_leaks("64B", filter_fn=filter_gc)
def test_handler_repr():
    handler = picologging.Handler()
    assert repr(handler) == "<Handler (NOTSET)>"

    handler = picologging.Handler(level=picologging.WARNING)
    assert repr(handler) == "<Handler (WARNING)>"
