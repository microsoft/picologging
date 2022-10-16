import pytest

import picologging
from picologging.config import dictConfig, valid_ident


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
    logger = picologging.getLogger("test_config.module")
    assert logger.name == "test_config.module"
    assert logger.level == picologging.INFO
    assert len(logger.handlers) == 1
    assert logger.handlers[0].name == "console"
    assert isinstance(logger.handlers[0], picologging.StreamHandler)

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


def test_valid_ident():
    assert valid_ident("test")
    with pytest.raises(ValueError):
        valid_ident("test.test")
    with pytest.raises(ValueError):
        valid_ident("test test")
    with pytest.raises(ValueError):
        valid_ident("test-test")


def test_configure_with_filters():
    config = {
        "version": 1,
        "loggers": {
            "test_config": {
                "handlers": ["console"],
                "level": "INFO",
            },
        },
        "formatters": {
            "standard": {
                "format": "%(asctime)s %(levelname)s %(name)s::%(message)s",
                "validate": True,
            },
        },
        "handlers": {
            "console": {
                "class": "picologging.StreamHandler",
                "filters": ["test_filter"],
                "formatter": "standard",
            },
        },
        "filters": {
            "test_filter": {
                "()": "picologging.Filter",
                "name": "test_filter",
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
    assert len(logger.handlers[0].filters) == 1
    assert logger.handlers[0].filters[0].name == "test_filter"
    assert isinstance(logger.handlers[0].filters[0], picologging.Filter)
    assert (
        logger.handlers[0].formatter._fmt
        == "%(asctime)s %(levelname)s %(name)s::%(message)s"
    )


def test_configure_with_non_defined_handlers():
    config = {
        "version": 1,
        "loggers": {
            "test_config": {
                "handlers": ["potato"],
                "level": "INFO",
            },
        },
    }
    with pytest.raises(ValueError):
        dictConfig(config)


def test_config_existing_disabled_logger_90195():
    # See gh-90195
    config = {
        "version": 1,
        "disable_existing_loggers": False,
        "handlers": {
            "console": {
                "level": "DEBUG",
                "class": "logging.StreamHandler",
            },
        },
        "loggers": {"a": {"level": "DEBUG", "handlers": ["console"]}},
    }
    logger = picologging.getLogger("a")
    assert logger.disabled == False
    dictConfig(config)
    assert logger.disabled == False
    # Should disable all loggers ...
    dictConfig({"version": 1})
    assert logger.disabled == True
    del config["disable_existing_loggers"]
    dictConfig(config)
    # Logger should be enabled, since explicitly mentioned
    assert logger.disabled == False
