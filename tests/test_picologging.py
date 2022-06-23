import picologging
import pytest

levels = [
    (picologging.DEBUG, "DEBUG"),
    (picologging.INFO, "INFO"),
    (picologging.WARNING, "WARNING"),
    (picologging.ERROR, "ERROR"),
    (picologging.CRITICAL, "CRITICAL"),
    (picologging.NOTSET, "NOTSET"),
]

@pytest.mark.parametrize("level, level_name", levels)
def test_getlevelname(level, level_name):
    assert picologging.getLevelName(level) == level_name

