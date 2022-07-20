import picologging
from io import StringIO


def run_profile(level=picologging.DEBUG):
    logger = picologging.Logger("test", level)
    tmp = StringIO()

    handler = picologging.StreamHandler(tmp)
    handler.setLevel(level)
    formatter = picologging.Formatter("%(name)s - %(levelname)s - %(message)s")
    handler.setFormatter(formatter)
    logger.handlers.append(handler)

    for _ in range(100_000):
        logger.debug("There has been a picologging issue %s %s %s", 1, 2, 3)

    assert tmp.getvalue() != ""


run_profile()
