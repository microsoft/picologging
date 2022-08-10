import io
import picologging
import logging
import pytest


def test_logger_attributes():
    logger = picologging.Logger("test")
    assert logger.name == "test"
    assert logger.level == logging.NOTSET
    assert logger.parent is None
    assert logger.propagate == True
    assert logger.handlers == []
    assert logger.disabled == False
    assert logger.propagate == True


level_names = [
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR",
    "CRITICAL",
    "NOTSET",
]

levels = [
    logging.DEBUG,
    logging.INFO,
    logging.WARNING,
    logging.ERROR,
    logging.CRITICAL,
    logging.NOTSET,
]


@pytest.mark.parametrize("level", levels)
def test_logging_custom_level(level):
    logger = picologging.Logger("test", level)
    assert logger.level == level


def test_set_level():
    logger = picologging.Logger("test")
    logger.setLevel(logging.DEBUG)
    assert logger.level == logging.DEBUG


def test_get_effective_level():
    logger = picologging.Logger("test")
    parent = picologging.Logger("parent", logging.DEBUG)
    logger.parent = parent
    assert logger.getEffectiveLevel() == logging.DEBUG
    assert logger.level == logging.NOTSET
    logger.setLevel(logging.WARNING)
    assert logger.getEffectiveLevel() == logging.WARNING


def test_dodgy_parents():
    logger = picologging.Logger("test")
    parent = "potato"
    with pytest.raises(TypeError):
        logger.parent = parent
        logger.getEffectiveLevel()


def test_add_filter():
    logger = picologging.Logger("test")
    filter = logging.Filter("filter1")
    logger.addFilter(filter)
    assert logger.filters == [filter]
    filter2 = logging.Filter("filter2")
    logger.addFilter(filter2)
    assert logger.filters == [filter, filter2]


def test_remove_filter():
    logger = picologging.Logger("test")
    filter = logging.Filter("filter1")
    logger.addFilter(filter)
    assert logger.filters == [filter]
    logger.removeFilter(filter)
    assert logger.filters == []


def test_no_filter():
    logger = picologging.Logger("test")
    record = picologging.LogRecord("test", logging.INFO, "test", 1, "test", (), {})
    assert logger.filter(record) == True


def test_filter_record():
    logger = picologging.Logger("test")
    filter = logging.Filter("hello")
    logger.addFilter(filter)
    record = picologging.LogRecord("hello", logging.INFO, "test", 1, "test", (), {})
    record2 = picologging.LogRecord("goodbye", logging.INFO, "test", 1, "test", (), {})
    assert logger.filter(record) == True
    assert logger.filter(record2) == False
    logger.removeFilter(filter)
    assert logger.filter(record) == True
    assert logger.filter(record2) == True


def test_filter_callable():
    logger = picologging.Logger("test")

    def filter(record):
        return record.name == "hello"

    logger.addFilter(filter)
    record = picologging.LogRecord("hello", logging.INFO, "test", 1, "test", (), {})
    assert logger.filter(record) == True
    record = picologging.LogRecord("goodbye", logging.INFO, "test", 1, "test", (), {})
    assert logger.filter(record) == False


def test_log_debug():
    logger = picologging.Logger("test", logging.DEBUG)
    stream = io.StringIO()
    handler = picologging.StreamHandler(stream)
    handler.setFormatter(picologging.Formatter("%(message)s"))
    logger.addHandler(handler)
    assert logger.debug("Hello World") is None
    result = stream.getvalue()
    assert result == "Hello World\n"


def test_log_debug_info_level_logger():
    logger = picologging.Logger("test", logging.INFO)
    stream = io.StringIO()
    logger.handlers.append(picologging.StreamHandler(stream))
    assert logger.debug("Hello World") is None
    result = stream.getvalue()
    assert result == ""


def test_log_debug_info_level_logger_logging_handler():
    logger = picologging.Logger("test", logging.INFO)
    stream = io.StringIO()
    logger.handlers.append(logging.StreamHandler(stream))
    assert logger.debug("Hello World") is None
    result = stream.getvalue()
    assert result == ""


@pytest.mark.parametrize("level", levels)
def test_log_log(level):
    logger = picologging.Logger("test", level)
    stream = io.StringIO()
    handler = picologging.StreamHandler(stream)
    handler.setFormatter(picologging.Formatter("%(message)s"))
    logger.addHandler(handler)
    assert logger.log(level, "Hello World") is None
    result = stream.getvalue()
    assert result == "Hello World\n"


def test_logger_with_explicit_level(capsys):
    logger = picologging.Logger("test", logging.DEBUG)
    tmp = io.StringIO()
    handler = picologging.StreamHandler(tmp)
    handler.setLevel(logging.DEBUG)
    formatter = picologging.Formatter("%(name)s - %(levelname)s - %(message)s")
    handler.setFormatter(formatter)
    logger.handlers.append(handler)
    logger.debug("There has been a picologging issue")
    result = tmp.getvalue()
    assert result == "test - DEBUG - There has been a picologging issue\n"
    cap = capsys.readouterr()
    assert cap.out == ""
    assert cap.err == ""


def test_exception_capture():
    logger = picologging.Logger("test", logging.DEBUG)
    tmp = io.StringIO()
    handler = picologging.StreamHandler(tmp)
    logger.addHandler(handler)
    try:
        1 / 0
    except ZeroDivisionError:
        logger.exception("bork")
    result = tmp.getvalue()
    assert "bork" in result
    assert "ZeroDivisionError: division by zero" in result


def test_getlogger_no_args():
    logger = logging.getLogger()
    assert logger.name == "root"
    assert logger.level == logging.WARNING
    assert logger.parent is None

    picologger = picologging.getLogger()
    assert picologger.name == "root"
    assert picologger.level == logging.WARNING
    assert picologger.parent is None


def test_logger_init_bad_args():
    with pytest.raises(TypeError):
        logger = picologging.Logger("goo", 10, dog=1)

    with pytest.raises(TypeError):
        logger = picologging.Logger(name="test", level="potato")


@pytest.mark.parametrize("level", levels)
def test_logger_repr(level):
    logger = picologging.Logger("test", level)
    assert repr(logger) == f"<Logger 'test' ({level_names[levels.index(level)]})>"


def test_logger_repr_effective_level():
    logger = picologging.Logger("test")
    logger.parent = picologging.Logger("parent", picologging.WARNING)
    assert repr(logger) == f"<Logger 'test' (WARNING)>"


def test_logger_repr_invalid_level():
    logger = picologging.Logger("test", level=100)
    assert repr(logger) == f"<Logger 'test' ()>"


def test_set_level_bad_type():
    logger = picologging.Logger("goo", picologging.DEBUG)
    with pytest.raises(TypeError):
        logger.setLevel("potato")


def test_add_remove_handlers():
    logger = picologging.Logger("goo", picologging.DEBUG)
    assert logger.handlers == []
    test_handler = picologging.Handler()
    logger.addHandler(test_handler)
    # add it twice should have no effect
    logger.addHandler(test_handler)
    assert len(logger.handlers) == 1
    assert test_handler in logger.handlers
    logger.removeHandler(test_handler)
    assert test_handler not in logger.handlers
    assert logger.handlers == []


@pytest.mark.parametrize("level_config", levels)
def test_log_and_handle(level_config):
    logger = picologging.Logger("test", level=level_config)
    tmp = io.StringIO()
    logger.addHandler(picologging.StreamHandler(tmp))
    assert not logger.info("info_message")
    assert not logger.debug("debug_message")
    assert not logger.warning("warning_message")
    assert not logger.error("error_message")
    assert not logger.fatal("fatal_message")
    assert not logger.critical("critical_message")
    assert not logger.log(level_config, "log_message")

    tmp_value = tmp.getvalue()

    if level_config <= picologging.DEBUG and level_config != picologging.NOTSET:
        assert "debug_message" in tmp_value
    else:
        assert "debug_message" not in tmp_value
    if level_config <= picologging.INFO and level_config != picologging.NOTSET:
        assert "info_message" in tmp_value
    else:
        assert "info_message" not in tmp_value
    if level_config <= picologging.WARNING and level_config != picologging.NOTSET:
        assert "warning_message" in tmp_value
    else:
        assert "warning_message" not in tmp_value
    if level_config <= picologging.ERROR and level_config != picologging.NOTSET:
        assert "error_message" in tmp_value
    else:
        assert "error_message" not in tmp_value
    if level_config <= picologging.FATAL and level_config != picologging.NOTSET:
        assert "fatal_message" in tmp_value
    else:
        assert "fatal_message" not in tmp_value
    if level_config <= picologging.CRITICAL and level_config != picologging.NOTSET:
        assert "critical_message" in tmp_value
    else:
        assert "critical_message" not in tmp_value
    assert "log_message" in tmp_value


def test_log_xx_bad_arguments():
    logger = picologging.Logger("test", level=picologging.DEBUG)

    with pytest.raises(TypeError):
        logger.info()
    with pytest.raises(TypeError):
        logger.debug()
    with pytest.raises(TypeError):
        logger.warning()
    with pytest.raises(TypeError):
        logger.error()
    with pytest.raises(TypeError):
        logger.fatal()
    with pytest.raises(TypeError):
        logger.critical()


def test_log_bad_arguments():
    logger = picologging.Logger("test")
    with pytest.raises(TypeError):
        logger.log("potato", "message")

    with pytest.raises(TypeError):
        logger.log()


def test_notset_parent_level_match():
    logger_child = picologging.Logger("child", picologging.NOTSET)
    logger_parent = picologging.Logger("parent", picologging.DEBUG)
    logger_child.parent = logger_parent

    parent_io = io.StringIO()
    child_io = io.StringIO()
    logger_child.addHandler(picologging.StreamHandler(child_io))
    logger_parent.addHandler(picologging.StreamHandler(parent_io))

    logger_child.info("child message")
    logger_parent.info("parent message")
    parent_value = parent_io.getvalue()
    child_value = child_io.getvalue()
    assert "child message" in child_value
    assert "child message" in parent_value
    assert "parent message" in parent_value
    assert "parent message" not in child_value


def test_error_parent_level():
    logger_child = picologging.Logger("child", picologging.WARNING)
    logger_parent = picologging.Logger("parent", picologging.ERROR)
    logger_child.parent = logger_parent

    parent_io = io.StringIO()
    child_io = io.StringIO()
    logger_child.addHandler(picologging.StreamHandler(child_io))
    logger_parent.addHandler(picologging.StreamHandler(parent_io))

    logger_child.info("info message")
    logger_child.warning("warning message")
    logger_child.error("error message")

    parent_value = parent_io.getvalue()
    child_value = child_io.getvalue()

    assert "info message" not in child_value
    assert "info message" not in parent_value
    assert "warning message" in child_value
    assert "warning message" in parent_value
    assert "error message" in child_value
    assert "error message" in parent_value


def test_nested_frame_stack():
    logger = picologging.Logger("test", level=picologging.DEBUG)
    tmp = io.StringIO()
    logger.addHandler(picologging.StreamHandler(tmp))

    def f():
        def g():
            logger.info("message", stack_info=True)

        g()

    f()
    result = tmp.getvalue()
    assert "message" in result
    assert " in g\n" in result
    assert " in f\n" in result


def test_exception_object_as_exc_info():
    e = Exception("arghhh!!")
    logger = picologging.Logger("test", level=picologging.DEBUG)
    tmp = io.StringIO()
    logger.addHandler(picologging.StreamHandler(tmp))
    logger.info("message", exc_info=e)
    result = tmp.getvalue()
    assert "message" in result
    assert "arghhh!!" in result


def test_logger_setlevel_resets_other_levels():
    stream = io.StringIO()
    handler = picologging.StreamHandler(stream)
    logger = picologging.getLogger("test")
    logger.addHandler(handler)
    logger.setLevel(picologging.WARNING)

    logger.debug("test")
    assert stream.getvalue() == ""

    logger.warning("test")
    assert stream.getvalue() == "test\n"

    logger.setLevel(picologging.ERROR)
    logger.warning("test")
    assert stream.getvalue() == "test\n"

    logger.error("test")
    assert stream.getvalue() == "test\ntest\n"
