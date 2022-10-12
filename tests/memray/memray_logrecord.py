import picologging
from io import StringIO


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

    for _ in range(100_000):
        logger.debug("There has been a picologging issue")
        logger.debug("There has been a picologging issue %s %s %s", 1, 2, 3)
        logger.info("There has been a picologging issue %s %s %s", 1, 2, 3)
        logger.warning("There has been a picologging issue %s %s %s", 1, 2, 3)

    assert len(tmp.getvalue()) > 100_000


if __name__ == "__main__":
    log()
