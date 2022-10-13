"""
Create 100,000  logger instances and run a log test on them
"""

from io import StringIO

import picologging


def log(level=picologging.INFO):
    logger = picologging.Logger("test", level)
    tmp = StringIO()

    handler = picologging.StreamHandler(tmp)
    handler.setLevel(level)
    formatter = picologging.Formatter(
        "%(created)f/%(asctime)s %(pathname)s:%(module)s:%(filename)s:%(lineno)d %(funcName)s %(levelno)d %(name)s - %(levelname)s %(process)d %(thread)d- %(message)s"
    )
    handler.setFormatter(formatter)
    logger.handlers.append(handler)

    logger.debug("There has been a picologging issue")


if __name__ == "__main__":
    for _ in range(100_000):
        log()
