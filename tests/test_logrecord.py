import threading
from picologging import LogRecord
import logging
import pytest
import os

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
    assert record.created


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

def test_no_args():
    record = LogRecord('hello', logging.WARNING, __file__, 123, 'bork boom', None, None)
    assert record.message == None
    assert record.getMessage() == "bork boom"
    assert record.message == "bork boom"

def test_no_args_and_format():
    record = LogRecord('hello', logging.WARNING, __file__, 123, 'bork %s', None, None)
    assert record.message == None
    assert record.getMessage() == "bork %s"
    assert record.message == "bork %s"

def test_repr():
    record = LogRecord('hello', logging.WARNING, __file__, 123, 'bork %s', (0, ), None)
    assert repr(record) == f"<LogRecord: hello, 30, {__file__}, 123, 'bork %s'>"

def test_mapping_dict():
    args = {
        "a": "b",
    }
    record = LogRecord('hello', logging.WARNING, __file__, 123, 'bork %s', (args, ), None)
    assert record.args == {"a": "b"}


def test_threading_info():
    record = LogRecord('hello', logging.WARNING, __file__, 123, 'bork', (), None)
    assert record.thread == threading.get_ident()
    assert record.threadName == None # Not supported


def test_process_info():
    record = LogRecord('hello', logging.WARNING, __file__, 123, 'bork', (), None)
    assert record.process == os.getpid()
    assert record.processName == None # Not supported
