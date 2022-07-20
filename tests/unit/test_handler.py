import picologging
import pytest


def test_basic_handler():
    handler = picologging.Handler()
    record = picologging.LogRecord(
        "test", picologging.INFO, "test", 1, "test", (), None, None, None
    )
    handler.handle(record)


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


def test_delete_handler():
    handler = picologging.Handler()
    del handler


def test_add_acquire_release():
    handler = picologging.Handler()
    handler.acquire()
    assert handler.release() is None


def test_init_with_name():
    handler = picologging.Handler(name="test")
    assert handler.name == "test"


def test_init_with_level():
    handler = picologging.Handler(level=picologging.DEBUG)
    assert handler.level == picologging.DEBUG


def test_get_set_name():
    handler = picologging.Handler(name="test")
    assert handler.get_name() == "test"
    handler.set_name("foo")
    assert handler.name == "foo"
    assert handler.get_name() == "foo"


def test_flush():
    handler = picologging.Handler()
    assert not handler.flush()


def test_close():
    handler = picologging.Handler()
    assert not handler.close()


def test_createLock():
    handler = picologging.Handler()
    assert not handler.createLock()


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


def test_set_level_nonint():
    handler = picologging.Handler()
    with pytest.raises(TypeError):
        handler.setLevel("potato")


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


def test_handle_error():
    handler = picologging.Handler()
    record = picologging.LogRecord(
        "test", picologging.INFO, __file__, 1, "test", (), None, None, None
    )
    assert not handler.handleError(record)
