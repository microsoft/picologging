from picologging import LogRecord
import logging


def test_logrecord_standard():
    record = LogRecord('hello', logging.WARNING, '/serv/', 123, 'bork bork bork', (), None)
    assert record.name == 'hello'
    assert record.msg == 'bork bork bork'
    assert record.levelno == logging.WARNING
    assert record.levelname == 'WARNING'
    assert record.pathname == '/serv/'
    assert record.args == ()

