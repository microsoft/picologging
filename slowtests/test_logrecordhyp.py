import logging
import logging.handlers
import queue
import sys

from flaky import flaky
from hypothesis import given
from hypothesis import strategies as st

import picologging
import picologging.handlers

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
def test_hypothesis_logrecord_constructor(
    name, level, lineno, msg, extra_arg, func, sinfo
):
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
    assert pico_record.args == stdl_record.args
    assert abs(pico_record.created - stdl_record.created) < 0.5
    assert pico_record.getMessage() == stdl_record.getMessage()


@flaky(max_runs=4, min_passes=1)
@given(
    name=st.text(),
    level=c_integers,
    lineno=c_integers,
    msg=st.text().filter(lambda t: t.find("%") < 0),
    extra_arg=st.text(),
    func=st.text(),
    sinfo=st.text(),
)
def test_hypothesis_logrecord_filename(
    name, level, lineno, msg, extra_arg, func, sinfo
):
    args = (extra_arg,)
    pico_record = picologging.LogRecord(
        name, level, __file__, lineno, msg + " %s", args, None, func, sinfo
    )
    stdl_record = logging.LogRecord(
        name, level, __file__, lineno, msg + " %s", args, None, func, sinfo
    )
    # Filename sometimes reported without extension on Windows
    assert pico_record.filename == stdl_record.filename


@given(args=st.lists(st.text(), min_size=0, max_size=10).map(tuple))
def test_hypothesis_logrecord_args(args):
    msg = " %s " * len(args)
    pico_record = picologging.LogRecord("", 10, __file__, 10, msg, args, None)
    stdl_record = logging.LogRecord("", 10, __file__, 10, msg, args, None)
    assert pico_record.msg == stdl_record.msg
    assert pico_record.args == stdl_record.args
    assert pico_record.getMessage() == stdl_record.getMessage()


@given(
    name=st.text(),
    level=c_integers,
    lineno=c_integers,
    msg=st.text().filter(lambda t: t.find("%") < 0),
    extra_arg=st.text(),
    func=st.text(),
    sinfo=st.text(),
)
def test_hypothesis_queuehandler_prepare(
    name, level, lineno, msg, extra_arg, func, sinfo
):
    """This test ensures the robustness of the prepare() method,
    which may be unstable in how it copies LogRecord using positional arguments
    (since it is currently not possible to use copy.copy).
    """
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
    pico_handler = picologging.handlers.QueueHandler(queue.Queue())
    stdl_handler = logging.handlers.QueueHandler(queue.Queue())
    pico_record2 = pico_handler.prepare(pico_record)
    stdl_record2 = stdl_handler.prepare(stdl_record)

    assert (
        pico_record2.name == pico_record.name == stdl_record.name == stdl_record2.name
    )
    assert (
        pico_record2.msg
        == stdl_record2.msg
        == stdl_handler.format(stdl_record)
        == pico_handler.format(pico_record)
    )
    assert (
        pico_record2.levelno
        == pico_record.levelno
        == stdl_record.levelno
        == stdl_record2.levelno
    )
    assert (
        pico_record2.lineno
        == pico_record.lineno
        == stdl_record.lineno
        == stdl_record2.lineno
    )
    assert (
        pico_record2.module
        == pico_record.module
        == stdl_record.module
        == stdl_record2.module
    )
    assert pico_record2.getMessage() == stdl_record2.getMessage()
