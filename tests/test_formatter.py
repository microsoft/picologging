from picologging import Formatter, LogRecord
import logging
import pytest


def test_formatter_default_fmt():
    f = Formatter()
    assert f.datefmt == None
    record = LogRecord('hello', logging.WARNING, __file__, 123, 'bork bork bork', (), None)
    s = f.format(record)
    assert s == 'bork bork bork'


def test_formatter_custom_fmt():
    f = Formatter("%(name)s %(levelname)s %(message)s")
    assert f.datefmt == None
    record = LogRecord('hello', logging.WARNING, __file__, 123, 'bork bork bork', (), None)
    s = f.format(record)
    assert s == 'hello WARNING bork bork bork'
    assert f.usesTime() == False
    assert f.formatMessage(record) == s


def test_formatter_default_fmt_against_builtin():
    pico_f = Formatter()
    logging_f = logging.Formatter()

    assert pico_f.datefmt == logging_f.datefmt

    pico_record = LogRecord('hello', logging.WARNING, __file__, 123, 'bork bork bork', (), None)
    logging_record = logging.LogRecord('hello', logging.WARNING, '/serv/', 123, 'bork bork bork', (), None)
    assert pico_f.format(pico_record) == logging_f.format(logging_record)
    assert pico_f.format(pico_record) == logging_f.format(pico_record)
    assert pico_f.formatMessage(pico_record) == logging_f.formatMessage(pico_record)

def test_formatter_custom_datefmt():
    f = Formatter("%(name)s %(levelname)s %(message)s", datefmt="%Y-%m-%d")
    assert f.datefmt == "%Y-%m-%d"
    record = LogRecord('hello', logging.WARNING, __file__, 123, 'bork bork bork', (), None)
    s = f.format(record)
    assert s == 'hello WARNING bork bork bork'

def test_formatter_explicit_none_datefmt_style():
    f = Formatter("%(name)s %(levelname)s %(message)s", None, '%')
    assert f.datefmt == None
    record = LogRecord('hello', logging.WARNING, __file__, 123, 'bork bork bork', (), None)
    s = f.format(record)
    assert s == 'hello WARNING bork bork bork'

possible_format_strings = [
    "%(message)s",
    "%(message)s is a potato",
    "%(name)s",
    "%(msg)s",
    "%(args)s",
    "%(levelname)s",
    "%(levelno)s",
    "%(pathname)s",
    "%(filename)s",
    "%(module)s",
    "%(funcName)s",
    "%(lineno)d",
    "%(threadName)s",
    "%(process)d",
    "%(processName)s",
    "%(stack_info)s",
    "%(exc_info)s",
    "%(exc_text)s",
]

@pytest.mark.parametrize("field", possible_format_strings)
def test_format_field(field):
    pico_f = Formatter(field)
    log_f = logging.Formatter(field)
    record = LogRecord('hello', logging.WARNING, __file__, 123, 'bork bork bork', (), None)
    assert pico_f.format(record) == log_f.format(record)


def test_format_time():
    pico_f = Formatter("%(msecs)d %(relativeCreated)d")
    record = LogRecord('hello', logging.WARNING, __file__, 123, 'bork bork bork', (), None)
    assert pico_f.format(record)


def test_asctime_field():
    pico_f = Formatter("%(asctime)s")
    record = LogRecord('hello', logging.WARNING, __file__, 123, 'bork bork bork', (), None)
    assert pico_f.format(record)
    assert pico_f.usesTime()


def test_record_with_stack_info():
    pico_f = Formatter("%(message)s")
    record = LogRecord('hello', logging.WARNING, __file__, 123, 'bork bork bork', (), None, None, "hello")
    assert pico_f.format(record) == "bork bork bork\nhello"

def test_format_stack():
    pico_f = Formatter("%(message)s")
    assert pico_f.formatStack([1,2,3]) == [1,2,3]

def test_delete_formatter():
    pico_f = Formatter("%(message)s")
    del pico_f