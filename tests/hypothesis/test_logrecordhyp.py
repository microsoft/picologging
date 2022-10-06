import logging
import sys

from hypothesis import given, reproduce_failure, strategies as st

import picologging

c_integers = st.integers().filter(lambda x: x < 2147483648 and x > -2147483649)


@given(
    name=st.text(),
    level=c_integers,
    lineno=c_integers,
    msg=st.text().filter(lambda t: t.find("%") < 0),
    extra_arg=st.text(),
    func=st.text(),
    sinfo=st.text(),
)
def test_hypothesis_logrecord_constructor(name, level, lineno, msg, extra_arg, func, sinfo):
    args = (extra_arg,)
    # Create an exception tuple
    exc_info = None
    try:
        10 / 0
    except ZeroDivisionError:
        exc_info = sys.exc_info()
    pico_record = picologging.LogRecord(
        name, level, __file__, lineno, msg + " %s", args, exc_info, func, sinfo
    )
    stdl_record = logging.LogRecord(
        name, level, __file__, lineno, msg + " %s", args, exc_info, func, sinfo
    )
    assert pico_record.name == stdl_record.name
    assert pico_record.msg == stdl_record.msg
    assert pico_record.levelno == stdl_record.levelno
    assert pico_record.lineno == stdl_record.lineno
    assert pico_record.module == stdl_record.module
    assert pico_record.filename == stdl_record.filename
    assert pico_record.args == stdl_record.args
    assert abs(pico_record.created - stdl_record.created) < 0.5
    assert pico_record.getMessage() == stdl_record.getMessage()


@given(args=st.lists(st.text(), min_size=0, max_size=10).map(tuple))
def test_hypothesis_logrecord_args(args):
    msg = " %s " * len(args)
    pico_record = picologging.LogRecord("", 10, __file__, 10, msg, args, None)
    stdl_record = logging.LogRecord("", 10, __file__, 10, msg, args, None)
    assert pico_record.msg == stdl_record.msg
    assert pico_record.args == stdl_record.args
    assert pico_record.getMessage() == stdl_record.getMessage()
