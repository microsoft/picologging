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
