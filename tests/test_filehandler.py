import os

import picologging
from picologging.handlers import WatchedFileHandler

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


def test_watchedfilehandler(tmp_path):
    log_file = tmp_path / 'log.txt'
    handler = WatchedFileHandler(log_file)
    logger = picologging.getLogger('test')
    logger.setLevel(picologging.DEBUG)
    logger.addHandler(handler)
    logger.warning('test')
    handler.close()

    with open(log_file, 'r') as f:
        assert f.read() == "test\n"


def test_watchedfilehandler_file_changed(tmp_path):
    log_file = tmp_path / 'log.txt'
    handler = WatchedFileHandler(log_file)
    logger = picologging.getLogger('test')
    logger.setLevel(picologging.DEBUG)
    logger.addHandler(handler)

    os.remove(log_file)
    with open(log_file, 'w'): ...

    logger.warning('test')
    handler.close()

    with open(log_file, 'r') as f:
        assert f.read() == "test\n"


def test_watchedfilehandler_file_removed(tmp_path):
    log_file = tmp_path / 'log.txt'
    handler = WatchedFileHandler(log_file)
    logger = picologging.getLogger('test')
    logger.setLevel(picologging.DEBUG)
    logger.addHandler(handler)

    os.remove(log_file)

    logger.warning('test')
    handler.close()

    with open(log_file, 'r') as f:
        assert f.read() == "test\n"
