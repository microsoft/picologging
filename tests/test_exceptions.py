import picologging

picologging.basicConfig()


def test_picologging_handler(capsys):
    logger = picologging.Logger('test')
    try:
        1/0
    except ZeroDivisionError:
        logger.exception("bork")
    cap = capsys.readouterr()
    result = cap.err + cap.out
    assert "bork" in result
    assert "ZeroDivisionError: division by zero" in result
