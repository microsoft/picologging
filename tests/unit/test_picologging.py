import sys
import picologging
import logging
import pytest

levels = [
    (picologging.DEBUG, "DEBUG"),
    (picologging.INFO, "INFO"),
    (picologging.WARNING, "WARNING"),
    (picologging.ERROR, "ERROR"),
    (picologging.CRITICAL, "CRITICAL"),
    (picologging.NOTSET, "NOTSET"),
]


@pytest.mark.parametrize("level, level_name", levels)
def test_getlevelname(level, level_name):
    assert picologging.getLevelName(level) == level_name


def test_getlevelname_invalid_level():
    assert picologging.getLevelName(100) == ""

    with pytest.raises(TypeError):
        picologging.getLevelName("100")


def test_root_logger_critical(capsys):
    picologging.root.handlers = []
    picologging.critical("test")

    cap = capsys.readouterr()
    assert cap.out == ""
    assert cap.err == "CRITICAL:root:test\n"


def test_root_logger_fatal(capsys):
    picologging.root.handlers = []
    picologging.fatal("test")

    cap = capsys.readouterr()
    assert cap.out == ""
    assert cap.err == "CRITICAL:root:test\n"


def test_root_logger_error(capsys):
    picologging.root.handlers = []
    picologging.error("test")

    cap = capsys.readouterr()
    assert cap.out == ""
    assert cap.err == "ERROR:root:test\n"


def test_root_logger_exception(capsys):
    picologging.root.handlers = []
    picologging.exception("test", exc_info=Exception("bork bork bork"))

    cap = capsys.readouterr()
    assert cap.out == ""
    assert cap.err == "ERROR:root:test\nException: bork bork bork\n"


def test_root_logger_warning(capsys):
    picologging.root.handlers = []
    picologging.warning("test")

    cap = capsys.readouterr()
    assert cap.out == ""
    assert cap.err == "WARNING:root:test\n"


def test_root_logger_warn(capsys):
    picologging.root.handlers = []
    picologging.warn("test")

    cap = capsys.readouterr()
    assert cap.out == ""
    assert cap.err == "WARNING:root:test\n"


def test_root_logger_info(capsys):
    picologging.root.handlers = []
    picologging.info("test")

    cap = capsys.readouterr()
    assert cap.out == ""
    assert cap.err == ""


def test_root_logger_debug(capsys):
    picologging.root.handlers = []
    picologging.debug("test")

    cap = capsys.readouterr()
    assert cap.out == ""
    assert cap.err == ""


def test_root_logger_log():
    picologging.root.handlers = []
    picologging.log(picologging.DEBUG, "test")


def test_basic_config_with_stream_and_filename_without_handlers():
    picologging.root.handlers = []

    with pytest.raises(ValueError):
        picologging.basicConfig(stream=sys.stderr, filename="log.txt")


def test_basic_config_with_stream_or_filename_with_handlers():
    handler = picologging.StreamHandler(sys.stderr)

    with pytest.raises(ValueError):
        picologging.basicConfig(handlers=[handler], stream=sys.stdout)


def test_basic_config_invalid_style():
    with pytest.raises(ValueError):
        picologging.basicConfig(style="!")


def test_basic_config_with_level():
    picologging.basicConfig(level=picologging.INFO)
    assert picologging.root.level == picologging.INFO


def test_basic_config_invalid_arguments():
    picologging.root.handlers = []
    with pytest.raises(ValueError):
        picologging.basicConfig(invalid_argument="value")


def test_log_record_factory():
    factory = picologging.getLogRecordFactory()

    assert factory == picologging.LogRecord

    picologging.setLogRecordFactory(logging.LogRecord)
    factory = picologging.getLogRecordFactory()

    assert factory == logging.LogRecord

    picologging.setLogRecordFactory(picologging.LogRecord)


def test_make_log_record():
    log_record = picologging.makeLogRecord({"levelno": picologging.WARNING})

    assert log_record.levelno == picologging.WARNING
