import logging
import picologging
logging.basicConfig()

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
