import io
import picologging
import logging
import pytest

def test_logger_attributes():
    logger = picologging.Logger('test')
    assert logger.name == 'test'
    assert logger.level == logging.NOTSET
    assert logger.parent == None
    assert logger.propagate == True
    assert logger.handlers == []
    assert logger.disabled == False
    assert logger.propagate == True


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
    logger = picologging.Logger('test', level)
    assert logger.level == level


def test_set_level():
    logger = picologging.Logger('test')
    logger.setLevel(logging.DEBUG)
    assert logger.level == logging.DEBUG


def test_get_effective_level():
    logger = picologging.Logger('test')
    parent = picologging.Logger('parent', logging.DEBUG)
    logger.parent = parent
    assert logger.getEffectiveLevel() == logging.DEBUG
    assert logger.level == logging.NOTSET
    logger.setLevel(logging.WARNING)
    assert logger.getEffectiveLevel() == logging.WARNING


def test_dodgy_parents():
    logger = picologging.Logger('test')
    parent = "potato"
    logger.parent = parent
    with pytest.raises(AttributeError):
        logger.getEffectiveLevel()


def test_add_filter():
    logger = picologging.Logger('test')
    filter = logging.Filter("filter1")
    logger.addFilter(filter)
    assert logger.filters == [filter]
    filter2 = logging.Filter("filter2")
    logger.addFilter(filter2)
    assert logger.filters == [filter, filter2]


def test_remove_filter():
    logger = picologging.Logger('test')
    filter = logging.Filter("filter1")
    logger.addFilter(filter)
    assert logger.filters == [filter]
    logger.removeFilter(filter)
    assert logger.filters == []


def test_no_filter():
    logger = picologging.Logger('test')
    record = picologging.LogRecord('test', logging.INFO, 'test', 1, 'test', (), {})
    assert logger.filter(record) == True


def test_filter_record():
    logger = picologging.Logger('test')
    filter = logging.Filter("hello")
    logger.addFilter(filter)
    record = picologging.LogRecord('hello', logging.INFO, 'test', 1, 'test', (), {})
    record2 = picologging.LogRecord('goodbye', logging.INFO, 'test', 1, 'test', (), {})
    assert logger.filter(record) == True
    assert logger.filter(record2) == False
    logger.removeFilter(filter)
    assert logger.filter(record) == True
    assert logger.filter(record2) == True


def test_filter_callable():
    logger = picologging.Logger('test')
    def filter(record):
        return record.name == 'hello'
    logger.addFilter(filter)
    record = picologging.LogRecord('hello', logging.INFO, 'test', 1, 'test', (), {})
    assert logger.filter(record) == True
    record = picologging.LogRecord('goodbye', logging.INFO, 'test', 1, 'test', (), {})
    assert logger.filter(record) == False


def test_log_debug():
    logger = picologging.Logger('test', logging.DEBUG)
    stream = io.StringIO()
    handler = picologging.StreamHandler(stream)
    handler.setFormatter(picologging.Formatter('%(message)s'))
    logger.addHandler(handler)
    assert logger.debug("Hello World") == None
    result = stream.getvalue()
    assert result == "Hello World\n"

def test_log_debug_info_level_logger():
    logger = picologging.Logger('test', logging.INFO)
    stream = io.StringIO()
    logger.handlers.append(picologging.StreamHandler(stream))
    assert logger.debug("Hello World") == None
    result = stream.getvalue()
    assert result == ""

def test_log_debug_info_level_logger_logging_handler():
    logger = picologging.Logger('test', logging.INFO)
    stream = io.StringIO()
    logger.handlers.append(logging.StreamHandler(stream))
    assert logger.debug("Hello World") == None
    result = stream.getvalue()
    assert result == ""

@pytest.mark.parametrize("level", levels)
def test_log_log(level):
    logger = picologging.Logger('test', level)
    stream = io.StringIO()
    handler = picologging.StreamHandler(stream)
    handler.setFormatter(picologging.Formatter('%(message)s'))
    logger.addHandler(handler)
    assert logger.log(level, "Hello World") == None
    result = stream.getvalue()
    assert result == "Hello World\n"


def test_logger_with_explicit_level(capsys):
    logger = picologging.Logger("test", logging.DEBUG)
    tmp = io.StringIO()
    handler = picologging.StreamHandler(tmp)
    handler.setLevel(logging.DEBUG)
    formatter = picologging.Formatter('%(name)s - %(levelname)s - %(message)s')
    handler.setFormatter(formatter)
    logger.handlers.append(handler)
    logger.debug("There has been a picologging issue")
    result = tmp.getvalue()
    assert result == "test - DEBUG - There has been a picologging issue\n"
    cap = capsys.readouterr()
    assert cap.out == ""
    assert cap.err == ""
