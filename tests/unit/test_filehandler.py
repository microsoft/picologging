import os
import platform

import pytest

import picologging
from picologging.handlers import WatchedFileHandler, RotatingFileHandler



def test_filehandler(tmp_path):
    log_file = tmp_path / 'log.txt'
    handler = picologging.FileHandler(log_file)
    logger = picologging.getLogger('test')
    logger.setLevel(picologging.DEBUG)
    logger.addHandler(handler)
    logger.warning('test')
    handler.close()

    with open(log_file, 'r') as f:
        assert f.read() == "test\n"


def test_filehandler_delay(tmp_path):
    log_file = tmp_path / 'log.txt'
    handler = picologging.FileHandler(log_file, delay=True)
    logger = picologging.getLogger('test')
    logger.setLevel(picologging.DEBUG)
    logger.addHandler(handler)
    logger.warning('test')
    handler.close()

    with open(log_file, 'r') as f:
        assert f.read() == "test\n"


@pytest.mark.skipif(platform.system() == "Windows", reason="Not supported on Windows.")
def test_watchedfilehandler(tmp_path):
    log_file = tmp_path / "log.txt"
    handler = WatchedFileHandler(log_file)
    logger = picologging.getLogger("test")
    logger.setLevel(picologging.DEBUG)
    logger.addHandler(handler)
    logger.warning("test")
    handler.close()

    with open(log_file, "r") as f:
        assert f.read() == "test\n"


@pytest.mark.skipif(platform.system() == "Windows", reason="Not supported on Windows.")
def test_watchedfilehandler_file_changed(tmp_path):
    log_file = tmp_path / "log.txt"
    handler = WatchedFileHandler(log_file)
    logger = picologging.getLogger("test")
    logger.setLevel(picologging.DEBUG)
    logger.addHandler(handler)

    os.remove(log_file)
    with open(log_file, "w"):
        ...

    logger.warning("test")
    handler.close()

    with open(log_file, "r") as f:
        assert f.read() == "test\n"


@pytest.mark.skipif(platform.system() == "Windows", reason="Not supported on Windows.")
def test_watchedfilehandler_file_removed(tmp_path):
    log_file = tmp_path / "log.txt"
    handler = WatchedFileHandler(log_file)
    logger = picologging.getLogger("test")
    logger.setLevel(picologging.DEBUG)
    logger.addHandler(handler)

    os.remove(log_file)

    logger.warning("test")
    handler.close()

    with open(log_file, "r") as f:
        assert f.read() == "test\n"


def test_rotatingfilehandler(tmp_path):
    log_file = tmp_path / "log.txt"
    handler = RotatingFileHandler(log_file, maxBytes=1, backupCount=2)
    logger = picologging.getLogger("test")
    logger.setLevel(picologging.DEBUG)
    logger.addHandler(handler)

    for _ in range(5):
        logger.warning("test")

    with open(log_file, "r") as f:
        assert f.read() == "test\n"

    for i in range(1, 3):
        log_file = tmp_path / f"log.txt.{i}"

        with open(log_file, "r") as f:
            assert f.read() == "test\n"

