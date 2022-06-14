from picologging import LogRecord
import logging


def test_logrecord_standard():
    record = LogRecord('hello', logging.WARNING, '/example/foo.py', 123, 'bork bork bork', (), None)
    assert record.name == 'hello'
    assert record.msg == 'bork bork bork'
    assert record.levelno == logging.WARNING
    assert record.levelname == 'WARNING'
    assert record.pathname == '/example/foo.py'
    assert record.module == 'foo'
    assert record.filename == 'foo.py'
    assert record.args == ()

def test_logrecord_logging_standard():
    record = logging.LogRecord('hello', logging.WARNING, '/example/foo.py', 123, 'bork bork bork', (), None)
    assert record.name == 'hello'
    assert record.msg == 'bork bork bork'
    assert record.levelno == logging.WARNING
    assert record.levelname == 'WARNING'
    assert record.pathname == '/example/foo.py'
    assert record.module == 'foo'
    assert record.filename == 'foo.py'
    assert record.args == ()