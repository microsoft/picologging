from picologging import PercentStyle, LogRecord, INFO
import pytest

def test_percentstyle():
    perc = PercentStyle("%(msg)s %(levelno)d %(name)s")
    record = LogRecord("test", INFO, __file__, 1, "hello", (), None, None, None)
    assert perc.format(record) == "hello 20 test"

def test_percentstyle_format_bad_argument():
    perc = PercentStyle("%(msg)s %(levelno)d %(name)s")
    with pytest.raises(AttributeError):
        perc.format(None) 
    with pytest.raises(AttributeError):
        perc.format("") 
    with pytest.raises(AttributeError):
        perc.format({})

def test_custom_attribute():
    perc = PercentStyle("%(custom)s")
    record = LogRecord("test", INFO, __file__, 1, "hello", (), None, None, None)
    record.custom = "custom"
    assert perc.format(record) == "custom"
