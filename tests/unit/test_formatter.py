import datetime
import io
import logging
import sys
import traceback
from logging import Formatter as LoggingFormatter

import pytest
from utils import filter_gc

from picologging import Formatter, LogRecord


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_formatter_default_fmt():
    f = Formatter()
    assert f.datefmt is None
    record = LogRecord(
        "hello", logging.WARNING, __file__, 123, "bork bork bork", (), None
    )
    s = f.format(record)
    assert s == "bork bork bork"


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_formatter_custom_fmt():
    f = Formatter("%(name)s %(levelname)s %(message)s")
    assert f.datefmt is None
    record = LogRecord(
        "hello", logging.WARNING, __file__, 123, "bork bork bork", (), None
    )
    s = f.format(record)
    assert s == "hello WARNING bork bork bork"
    assert f.usesTime() is False
    assert f.formatMessage(record) == s


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_formatter_default_fmt_against_builtin():
    pico_f = Formatter()
    logging_f = logging.Formatter()

    assert pico_f.datefmt == logging_f.datefmt

    pico_record = LogRecord(
        "hello", logging.WARNING, __file__, 123, "bork bork bork", (), None
    )
    logging_record = logging.LogRecord(
        "hello", logging.WARNING, "/serv/", 123, "bork bork bork", (), None
    )
    assert pico_f.format(pico_record) == logging_f.format(logging_record)
    assert pico_f.format(pico_record) == logging_f.format(pico_record)
    assert pico_f.formatMessage(pico_record) == logging_f.formatMessage(pico_record)


@pytest.mark.limit_leaks("256B", filter_fn=filter_gc)
def test_formatter_default_fmt_exc_info_against_builtin():
    pico_f = Formatter()
    logging_f = logging.Formatter()

    try:
        raise Exception("error")
    except Exception:
        exc_info = sys.exc_info()

    pico_record = LogRecord(
        "hello", logging.WARNING, __file__, 123, "bork bork bork", (), exc_info, "", ""
    )
    logging_record = logging.LogRecord(
        "hello", logging.WARNING, "/serv/", 123, "bork bork bork", (), exc_info, "", ""
    )
    assert pico_f.format(pico_record) == logging_f.format(logging_record)
    assert pico_f.format(pico_record) == logging_f.format(pico_record)
    assert pico_f.formatMessage(pico_record) == logging_f.formatMessage(pico_record)


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_formatter_custom_datefmt():
    f = Formatter("%(name)s %(levelname)s %(message)s", datefmt="%Y-%m-%d")
    assert f.datefmt == "%Y-%m-%d"
    record = LogRecord(
        "hello", logging.WARNING, __file__, 123, "bork bork bork", (), None
    )
    s = f.format(record)
    assert s == "hello WARNING bork bork bork"


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_formatter_explicit_none_datefmt_style():
    f = Formatter("%(name)s %(levelname)s %(message)s", None, "%")
    assert f.datefmt is None
    record = LogRecord(
        "hello", logging.WARNING, __file__, 123, "bork bork bork", (), None
    )
    s = f.format(record)
    assert s == "hello WARNING bork bork bork"


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


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
@pytest.mark.parametrize("field", possible_format_strings)
def test_format_field(field):
    pico_f = Formatter(field)
    log_f = logging.Formatter(field)
    record = LogRecord(
        "hello", logging.WARNING, __file__, 123, "bork bork bork", (), None
    )
    assert pico_f.format(record) == log_f.format(record)


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_format_time():
    pico_f = Formatter("%(msecs)d %(relativeCreated)d")
    record = LogRecord(
        "hello", logging.WARNING, __file__, 123, "bork bork bork", (), None
    )
    assert pico_f.format(record)


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_asctime_field():
    pico_f = Formatter("%(asctime)s")
    record = LogRecord(
        "hello", logging.WARNING, __file__, 123, "bork bork bork", (), None
    )
    assert pico_f.format(record)
    assert pico_f.usesTime()


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_asctime_field_buffer():
    pico_f = Formatter("%(asctime)s")
    record = LogRecord(
        "hello", logging.WARNING, __file__, 123, "bork bork bork", (), None
    )
    logging_f = LoggingFormatter("%(asctime)s")

    assert pico_f.format(record).split(",")[0] == logging_f.format(record).split(",")[0]
    assert pico_f.usesTime()


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_record_with_stack_info():
    pico_f = Formatter("%(message)s")
    record = LogRecord(
        "hello",
        logging.WARNING,
        __file__,
        123,
        "bork bork bork",
        (),
        None,
        None,
        "hello",
    )
    assert pico_f.format(record) == "bork bork bork\nhello"


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_record_with_non_str_sstack_info():
    pico_f = Formatter("%(message)s")
    record = LogRecord(
        "hello",
        logging.WARNING,
        __file__,
        123,
        "bork bork bork",
        (),
        None,
        None,
        ["hello", "world"],
    )
    assert pico_f.format(record) == "bork bork bork\n['hello', 'world']"


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_format_stack():
    pico_f = Formatter("%(message)s")
    assert pico_f.formatStack([1, 2, 3]) == [1, 2, 3]


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_delete_formatter():
    pico_f = Formatter("%(message)s")
    del pico_f


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_formatter_bad_init_args():
    with pytest.raises(TypeError):
        Formatter(dog="good boy")


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_formatter_bad_style():
    with pytest.raises(ValueError):
        Formatter(style="!")


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_formatter_bad_style_type():
    with pytest.raises(TypeError):
        Formatter(style=123)


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_formatter_bad_fmt_type():
    record = LogRecord(
        "hello", logging.WARNING, __file__, 123, "bork bork bork", (), None
    )
    with pytest.raises((TypeError, ValueError)):
        f = Formatter(fmt=123)
        assert f.format(record) == "bork bork bork"


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_formatter_with_validate_flag_and_invalid_fmt():
    f = Formatter(fmt="%(message ", validate=True)
    record = LogRecord(
        "hello", logging.WARNING, __file__, 123, "bork bork bork", (), None
    )
    assert f.format(record) == "%(message "


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_datefmt_bad_type():
    with pytest.raises(TypeError):
        Formatter(datefmt=123)


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_format_with_custom_datefmt():
    actual_date = datetime.datetime.now().strftime("%Y-%m-%d")
    f = Formatter("%(name)s %(levelname)s %(message)s %(asctime)s", datefmt="%Y-%m-%d")
    assert f.datefmt == "%Y-%m-%d"
    record = LogRecord(
        "hello", logging.WARNING, __file__, 123, "bork bork bork", (), None
    )
    s = f.format(record)
    assert s == f"hello WARNING bork bork bork {actual_date}"
    assert f.usesTime() is True
    assert f.formatMessage(record) == s


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_format_with_custom_datefmt_and_microseconds_specifier():
    f = Formatter(
        "%(name)s %(levelname)s %(message)s %(asctime)s bork", datefmt="%F %T,%f"
    )
    assert f.datefmt == "%F %T,%f"
    record = LogRecord(
        "hello", logging.WARNING, __file__, 123, "bork bork bork", (), None
    )
    s = f.format(record)
    actual_date = datetime.datetime.fromtimestamp(record.created).strftime("%F %T,%f")
    assert s == f"hello WARNING bork bork bork {actual_date} bork"
    assert f.usesTime() is True
    assert f.formatMessage(record) == s


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_formatter_repr():
    f = Formatter("%(message)s")
    assert repr(f) == "<Formatter: fmt='%(message)s'>"


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_exc_info_invalid_type():
    record = LogRecord(
        "hello", logging.WARNING, __file__, 123, "bork bork bork", (), (1, 2, 3)
    )
    f = Formatter()
    with pytest.raises(AttributeError):
        f.format(record)


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_exc_info_invalid_value_types():
    record = LogRecord(
        "hello", logging.WARNING, __file__, 123, "bork bork bork", (), [1, 2, 3]
    )
    f = Formatter()
    with pytest.raises(TypeError):
        f.format(record)


# TODO #41 : test defaults are propagating to string formatters


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_formatter_templates():
    # Not supported, so check it doesn't just crash
    with pytest.raises(NotImplementedError):
        Formatter("%(message)s", style="$")


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_format_exception():
    pico_f = Formatter("%(message)s")

    try:
        raise Exception("error")
    except Exception:
        ei = sys.exc_info()

    result = pico_f.formatException(ei)
    assert result.startswith("Traceback (most recent call last):")
    assert result.endswith(
        'test_format_exception\n    raise Exception("error")\nException: error'
    )


@pytest.mark.limit_leaks("192B", filter_fn=filter_gc)
def test_override_format_exception():
    class CustomFormatter(Formatter):
        def formatException(self, ei) -> str:
            sio = io.StringIO()
            tb = ei[2]
            traceback.print_exception(ei[0], ei[1], tb, None, sio)
            s = sio.getvalue()
            sio.close()
            if s[-1:] == "\n":
                s = s[:-1]
            return "Custom " + s

    formatter = CustomFormatter("%(message)s")

    try:
        raise Exception("error")
    except Exception:
        ei = sys.exc_info()

    result = formatter.formatException(ei)
    assert result.startswith("Custom Traceback (most recent call last):")
    assert result.endswith(
        'test_override_format_exception\n    raise Exception("error")\nException: error'
    )
