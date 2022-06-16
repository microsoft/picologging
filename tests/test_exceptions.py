import logging
import picologging
import pytest

logging.basicConfig()

@pytest.fixture()
def picologging_enabled():
    picologging.install()
    yield
    picologging.uninstall()


def test_builtin_handler(capsys):
    logger = logging.Logger('test')
    try:
        1/0
    except ZeroDivisionError:
        logger.exception("bork")
    cap = capsys.readouterr()
    result = cap.err + cap.out
    assert "bork" in result
    assert "ZeroDivisionError: division by zero" in result


def test_picologging_handler(capsys, picologging_enabled):
    logger = logging.Logger('test')
    try:
        1/0
    except ZeroDivisionError:
        logger.exception("bork")
    cap = capsys.readouterr()
    result = cap.err + cap.out
    assert "bork" in result
    assert "ZeroDivisionError: division by zero" in result
