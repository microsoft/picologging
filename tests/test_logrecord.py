from picologging import LogRecord
import logging
import pytest

def test_logrecord_standard():
    record = LogRecord('hello', logging.WARNING, __file__, 123, 'bork bork bork', (), None)
    assert record.name == 'hello'
    assert record.msg == 'bork bork bork'
    assert record.levelno == logging.WARNING
    assert record.levelname == 'WARNING'
    assert record.pathname == __file__
    assert record.module == 'test_logrecord'
    assert record.filename == 'test_logrecord.py'
    assert record.args == ()


def test_logrecord_args():
    record = LogRecord('hello', logging.WARNING, __file__, 123, 'bork %s', ('boom'), None)
    assert record.name == 'hello'
    assert record.msg == 'bork %s'
    assert record.args == ('boom')
    assert record.message == None


def test_logrecord_getmessage_with_args():
    record = LogRecord('hello', logging.WARNING, __file__, 123, 'bork %s', ('boom'), None)
    assert record.message == None
    assert record.getMessage() == "bork boom"
    assert record.message == "bork boom"


def test_logrecord_getmessage_no_args():
    record = LogRecord('hello', logging.WARNING, __file__, 123, 'bork boom', (), None)
    assert record.message == None
    assert record.getMessage() == "bork boom"
    assert record.message == "bork boom"


def test_args_format_mismatch():
    record = LogRecord('hello', logging.WARNING, __file__, 123, 'bork boom %s %s', (0, ), None)
    assert record.message == None
    with pytest.raises(TypeError):
        record.getMessage()


def test_args_len_mismatch():
    record = LogRecord('hello', logging.WARNING, __file__, 123, 'bork boom %s', (0, 1, 2), None)
    assert record.message == None
    with pytest.raises(TypeError):
        record.getMessage()
