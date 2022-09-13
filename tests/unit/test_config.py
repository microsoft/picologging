import pytest
import picologging
from picologging.config import dictConfig


def test_dictconfig():
    class TestFilter(picologging.Filter):
        def __init__(self, param=None):
            self.param = param

        def filter(self, record):
            return True

    config = {
        "version": 1,
        "root": {"handlers": ["console"], "level": "DEBUG"},
        "loggers": {
            "test_config": {
                "handlers": ["console"],
                "level": "INFO",
                "propagate": True,
            },
        },
        "filters": {
            "test_filter": {"()": TestFilter},
            "example_filter": {},
        },
        "formatters": {
            "test_formatter": {"()": "picologging.Formatter"},
            "example_formatter": {"class": "picologging.Formatter"},
            "standard": {"format": "%(asctime)s [%(levelname)s] %(name)s: %(message)s"},
        },
        "handlers": {
            "console": {
                "class": "picologging.StreamHandler",
                "filters": ["test_filter"],
                "level": picologging.DEBUG,
            },
            "test": {
                "()": "picologging.StreamHandler",
                ".": {"level": picologging.DEBUG},
            },
            "console_formatted": {
                "class": "picologging.StreamHandler",
                "formatter": "standard",
            },
        },
    }

    dictConfig(config)

    root = picologging.getLogger()
    assert root.name == "root"
    assert root.level == picologging.DEBUG
    assert root.handlers[0].name == "console"
    assert isinstance(root.handlers[0], picologging.StreamHandler)

    logger = picologging.getLogger("test_config")
    assert logger.name == "test_config"
    assert logger.level == picologging.INFO
    assert logger.handlers[0].name == "console"
    assert isinstance(logger.handlers[0], picologging.StreamHandler)

    # Reset root logger
    picologging.root.handlers = []
    picologging.root.setLevel(picologging.WARNING)


def test_dictconfig_clear_existing_loggers(tmp_path):
    log_file = tmp_path / "log.txt"
    handler = picologging.FileHandler(log_file)
    logger = picologging.getLogger("test_config")
    logger.setLevel(picologging.DEBUG)
    logger.addHandler(handler)

    config = {
        "version": 1,
        "loggers": {
            "test_config": {
                "handlers": ["console"],
                "level": "INFO",
            },
        },
        "handlers": {
            "console": {
                "class": "picologging.StreamHandler",
            },
        },
    }

    dictConfig(config)

    logger = picologging.getLogger("test_config")
    assert logger.name == "test_config"
    assert logger.level == picologging.INFO
    assert len(logger.handlers) == 1
    assert logger.handlers[0].name == "console"
    assert isinstance(logger.handlers[0], picologging.StreamHandler)


def test_dictconfig_config_exceptions():
    with pytest.raises(ValueError):
        dictConfig({})

    with pytest.raises(ValueError):
        dictConfig({"version": 0})

    config = {
        "version": 1,
        "handlers": {
            "console": {
                "class": "picologging.SomeHandler",
            },
        },
    }

    with pytest.raises(ValueError):
        dictConfig(config)

    config = {
        "version": 1,
        "loggers": {
            "test_config": {"handlers": ["console"]},
        },
    }

    with pytest.raises(ValueError):
        dictConfig(config)


def test_config_exception_invalid_filter_for_handler():
    config = {
        "version": 1,
        "handlers": {
            "console": {
                "class": "picologging.StreamHandler",
                "filters": ["test_filter"],
                "level": picologging.DEBUG,
            },
        },
    }

    with pytest.raises(ValueError):
        dictConfig(config)


def test_dictconfig_incremental_not_supported():
    config = {"version": 1, "incremental": True}

    with pytest.raises(ValueError):
        dictConfig(config)


def test_dictconfig_formatters_exception():
    config = {
        "version": 1,
        "formatters": {
            "example_formatter": {"class": "picologging.NoFormatter"},
        },
    }

    with pytest.raises(ValueError):
        dictConfig(config)


def test_dictconfig_filters_exception():
    config = {
        "version": 1,
        "filters": {
            "example_filters": {"()": "picologging.NoFilter"},
        },
    }

    with pytest.raises(ValueError):
        dictConfig(config)


def test_reconfigure_dictconfig_with_child_loggers():
    logger = picologging.getLogger("test_config")
    logger.addHandler(picologging.StreamHandler())

    config = {
        "version": 1,
        "loggers": {
            "test_config.module": {
                "handlers": ["console"],
                "level": "INFO",
            },
        },
        "handlers": {
            "console": {
                "class": "picologging.StreamHandler",
            },
        },
    }

    dictConfig(config)

    config = {
        "version": 1,
        "loggers": {
            "test_config": {
                "handlers": ["console"],
                "level": "INFO",
            },
        },
        "handlers": {
            "console": {
                "class": "picologging.StreamHandler",
            },
        },
    }

    dictConfig(config)
