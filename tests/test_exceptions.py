import picologging
import io

picologging.basicConfig()


def test_picologging_handler(capsys):
    logger = picologging.getLogger(__name__)
    tmp = io.StringIO()
    handler = picologging.StreamHandler(tmp)
    logger.addHandler(handler)
    try:
        1/0
    except ZeroDivisionError:
        logger.exception("bork")
    result = tmp.getvalue()
    assert "bork" in result
    assert "ZeroDivisionError: division by zero" in result
