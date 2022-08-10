from picologging.config import dictConfig


def test_dict_config():
    config = {
        "version": 1,
        "disable_existing_loggers": False,
        "loggers": {
            "": {
                "level": "INFO",
            },
            "another.module": {
                "level": "DEBUG",
            },
        },
    }

    dictConfig(config)
