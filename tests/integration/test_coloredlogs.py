import io

try:
    import coloredlogs

    has_libs = True
except ImportError:
    has_libs = False

import pytest

import picologging as logging


@pytest.mark.skipif(not has_libs, reason="Missing libraries")
def test_coloredlogs_logger():
    # Setup colored logger
    stream = io.StringIO()

    logger = logging.getLogger()
    logger.addHandler(logging.StreamHandler(stream))
    coloredlogs.install(logger=logger, level="INFO")

    logger.info("Info message")
    logger.warning("Warning message")
    logger.error("Error message")
    logger.critical("Critical message")

    log = stream.getvalue()
    assert "Info message" in log
    assert "Warning message" in log
    assert "Error message" in log
    assert "Critical message" in log
