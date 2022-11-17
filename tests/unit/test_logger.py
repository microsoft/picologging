import io
import logging
import uuid

import pytest

import picologging


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


def test_custom_logger_has_no_parent():
    logger = picologging.Logger("test")
    assert logger.parent is None


def test_remove_non_existent_handler():
    logger = picologging.Logger("test")
    assert logger.removeHandler("handler") is None


def test_set_level():
    logger = picologging.Logger("test")
    logger.setLevel(logging.DEBUG)
    assert logger.level == logging.DEBUG


def test_disabled_logger():
    logger = picologging.Logger("test", logging.DEBUG)
    logger.disabled = True
    stream = io.StringIO()
    logger.handlers.append(picologging.StreamHandler(stream))
    assert logger.debug("Hello World") is None
    result = stream.getvalue()
    assert result == ""
    ex = Exception("arghhh!!")
    logger.exception("Hello World", ex)
    result = stream.getvalue()
    assert result == ""


def test_logger_with_logging_handler():
    logger = picologging.Logger("test", logging.DEBUG)
    stream = io.StringIO()
    handler = logging.StreamHandler(stream)
    handler.setFormatter(logging.Formatter("%(message)s"))
    logger.addHandler(handler)
    assert logger.debug("Hello World") is None
    result = stream.getvalue()
    assert result == "Hello World\n"


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

    with pytest.raises(TypeError):
        logger.parent = logging.getLogger("test")
        logger.getEffectiveLevel()

    with pytest.raises(TypeError):
        del logger.parent


def test_add_filter():
    logger = picologging.Logger("test")
    filter = picologging.Filter("filter1")
    logger.addFilter(filter)
    assert logger.filters == [filter]
    filter2 = picologging.Filter("filter2")
    logger.addFilter(filter2)
    assert logger.filters == [filter, filter2]


def test_remove_filter():
    logger = picologging.Logger("test")
    filter = picologging.Filter("filter1")
    logger.addFilter(filter)
    assert logger.filters == [filter]
    logger.removeFilter(filter)
    assert logger.filters == []
    logger.removeFilter(filter)
    assert logger.filters == []


def test_delete_filter():
    filter = picologging.Filter("filter1")
    del filter


def test_filterer_direct_type():
    filterable = picologging.Filterer()
    assert filterable.filters == []
    filter = picologging.Filter("filter1")
    filterable.addFilter(filter)
    assert filterable.filters == [filter]
    filter2 = picologging.Filter("filter2")
    filterable.addFilter(filter2)
    assert filterable.filters == [filter, filter2]
    filterable.removeFilter(filter)
    assert filterable.filters == [filter2]
    filterable.removeFilter(filter2)
    assert filterable.filters == []
    del filterable


def test_no_filter():
    logger = picologging.Logger("test")
    record = picologging.LogRecord("test", logging.INFO, "test", 1, "test", (), {})
    assert logger.filter(record) == True


def test_filter_record():
    logger = picologging.Logger("test")
    filter = picologging.Filter("hello")
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


def test_getlogger_root_level():
    # Check that the root logger defaults to WARNING
    logger = picologging.getLogger()
    assert logger.getEffectiveLevel() == picologging.WARNING


def test_getlogger_nonroot_levels():
    """
    Check that descendant loggers get the root level on construction.

    Before:
    | Logger       | Level     | Effective Level |
    |--------------|-----------|-----------------|
    | root         | 30        | 30              |

    After:
    | child        | 0         | 30              |
    | grandchild   | 0         | 30              |
    """
    child_name = str(uuid.uuid4())
    child_logger = picologging.getLogger(str(uuid.uuid4()))
    assert child_logger.level == picologging.NOTSET
    assert child_logger.getEffectiveLevel() == picologging.WARNING

    grandchild_name = f"{child_name}.str(uuid.uuid4())"
    grandchild_logger = picologging.getLogger(grandchild_name)
    assert grandchild_logger.level == picologging.NOTSET
    assert grandchild_logger.getEffectiveLevel() == picologging.WARNING


def test_getlogger_parentchild_levels():
    """
    Check interaction of setLevel with logger hierarchy

    Before:
    | Logger       | Level     | Effective Level |
    |--------------|-----------|-----------------|
    | root         | 30        | 30              |
    | parent       | 0         | 30              |

    After parent.setLevel(INFO):
    | parent       | 20        | 20              |

    After construction of child logger:
    | child        | 0         | 20              |

    After child.setLevel(WARNING):
    | child        | 30        | 30              |
    """
    parent_name = str(uuid.uuid4())
    parent_logger = picologging.getLogger(parent_name)
    assert parent_logger.getEffectiveLevel() == picologging.WARNING

    parent_logger.setLevel(picologging.INFO)
    assert parent_logger.getEffectiveLevel() == picologging.INFO

    child_name = f"{parent_name}.{uuid.uuid4()}"
    child_logger = picologging.getLogger(child_name)
    assert child_logger.getEffectiveLevel() == picologging.INFO

    child_logger.setLevel(picologging.WARNING)
    assert child_logger.getEffectiveLevel() == picologging.WARNING


def test_getlogger_setlevel_after():
    """
    Check for setting parent level after child construction

    Before:
    | Logger       | Level     | Effective Level |
    |--------------|-----------|-----------------|
    | root         | 30        | 30              |
    | parent       | 0         | 30              |

    After parent.setLevel(INFO):
    | parent       | 20        | 20              |

    After construction of child logger:
    | child        | 0         | 20              |

    After parent.setLevel(DEBUG):
    | parent       | 10        | 10              |
    | child        | 0         | 10              |
    """
    parent_name = str(uuid.uuid4())
    parent_logger = picologging.getLogger(parent_name)
    parent_logger.setLevel(picologging.WARNING)

    child_name = f"{parent_name}.{uuid.uuid4()}"
    child_logger = picologging.getLogger(child_name)

    parent_logger.setLevel(picologging.DEBUG)
    assert child_logger.getEffectiveLevel() == picologging.DEBUG


def test_getlogger_setlevel_after_multiple_children():
    """
    Check for setting parent level after child construction

    Before:
    | Logger       | Level     | Effective Level |
    |--------------|-----------|-----------------|
    | root         | 30        | 30              |
    | parent       | 0         | 30              |

    After parent.setLevel(WARNING):
    | parent       | 30        | 30              |

    After construction of child loggers:
    | child1       | 0         | 30              |
    | child2       | 0         | 30              |

    After parent.setLevel(DEBUG):
    | parent       | 10        | 10              |
    | child1       | 0         | 10              |
    | child2       | 0         | 10              |
    """
    parent_name = str(uuid.uuid4())
    parent_logger = picologging.getLogger(parent_name)
    parent_logger.setLevel(picologging.WARNING)

    child1_name = f"{parent_name}.{uuid.uuid4()}"
    child2_name = f"{parent_name}.{uuid.uuid4()}"
    child1_logger = picologging.getLogger(child1_name)
    child2_logger = picologging.getLogger(child2_name)

    parent_logger.setLevel(picologging.DEBUG)
    assert child1_logger.getEffectiveLevel() == picologging.DEBUG
    assert child2_logger.getEffectiveLevel() == picologging.DEBUG


def test_getlogger_setlevel_message_handled():
    """
    Check for child creation before parent creation
    and appropriate handling of messages.

    Before:
    | Logger       | Level     | Effective Level |
    |--------------|-----------|-----------------|
    | root         | 30        | 30              |
    | child        | 0         | 30              |

    After construction of parent logger:
    | parent       | 0         | 30              |
    | child        | 0         | 30              |

    After parent.setLevel(DEBUG):
    | parent       | 10        | 10              |
    | child        | 0         | 10              |
    """
    parent_name = str(uuid.uuid4())
    child_name = f"{parent_name}.{uuid.uuid4()}"
    child_logger = picologging.getLogger(child_name)
    assert child_logger.level == picologging.NOTSET
    assert child_logger.getEffectiveLevel() == picologging.WARNING

    parent_logger = picologging.getLogger(parent_name)
    stream = io.StringIO()
    handler = picologging.StreamHandler(stream)
    child_logger.addHandler(handler)

    child_logger.log(picologging.DEBUG, "Hello World")
    assert stream.getvalue() == ""

    parent_logger.setLevel(picologging.DEBUG)
    assert child_logger.getEffectiveLevel() == picologging.DEBUG
    child_logger.log(picologging.DEBUG, "Hello World")
    assert stream.getvalue() == "Hello World\n"


def test_getlogger_with_placeholder_parent():
    # Logging levels when some parent does not exist yet.
    stream = io.StringIO()
    handler = picologging.StreamHandler(stream)

    top_logger = picologging.getLogger("A")
    top_logger.addHandler(handler)
    bottom1_logger = picologging.getLogger("A.B.C")
    bottom2_logger = picologging.getLogger("A.B.D")
    top_logger.setLevel(picologging.INFO)

    assert top_logger.getEffectiveLevel() == picologging.INFO
    assert bottom1_logger.getEffectiveLevel() == picologging.INFO
    assert bottom2_logger.getEffectiveLevel() == picologging.INFO

    # These logs should be handled
    top_logger.log(picologging.WARN, "TLW")
    top_logger.info("TLI")
    bottom1_logger.log(picologging.WARN, "BL1W")
    bottom1_logger.info("BL1I")
    bottom2_logger.log(picologging.WARN, "BL2W")
    bottom2_logger.info("BL2I")

    # These should not
    top_logger.log(picologging.DEBUG, "TLD")
    bottom1_logger.debug("BLD")

    assert stream.getvalue() == "TLW\nTLI\nBL1W\nBL1I\nBL2W\nBL2I\n"

    # Now breathe life into the placeholder
    middle_logger = picologging.getLogger("A.B")
    assert middle_logger.getEffectiveLevel() == picologging.INFO
