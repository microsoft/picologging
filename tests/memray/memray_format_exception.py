import sys

from picologging import Formatter


def test_format_exception():
    pico_f = Formatter("%(message)s")

    try:
        raise Exception("error")
    except Exception:
        ei = sys.exc_info()

    result = pico_f.formatException(ei)
    assert result.startswith("Traceback (most recent call last):")
    assert result.endswith(
        'test_format_exception\n    raise Exception("error")\nException: error'
    )


if __name__ == "__main__":
    for _ in range(100_000):
        test_format_exception()
