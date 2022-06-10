import logging
import picologging

def test_install_handlers():
    logger = logging.getLogger(__name__)
    n_handlers = len(logger.handlers)
    n_root_handlers = len(logger.root.handlers)
    
    picologging.install()
    assert len(logger.handlers) == n_handlers
    assert len(logger.root.handlers) == n_root_handlers

    picologging.uninstall()


def test_install_makerecord():
    picologging.install()
    logger = logging.getLogger()
    record = logger.makeRecord("test_name", logging.WARNING, "foo", 123, "the message", (), False)
    assert isinstance(record, picologging.LogRecord)
    picologging.uninstall()
