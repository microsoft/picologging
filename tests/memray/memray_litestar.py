from litestar import Litestar, Request, get
from litestar.logging import LoggingConfig
from litestar.testing import TestClient

logging_config = LoggingConfig(
    loggers={
        "app": {
            "level": "DEBUG",
            "handlers": ["queue_listener"],
            "propagate": False,
        }
    }
)


@get("/")
def hello_world(request: Request) -> dict[str, str]:
    """Handler function that returns a greeting dictionary."""
    request.logger.info("No results in response")
    request.logger.debug("doing things...")
    return {"hello": "world"}


app = Litestar(
    route_handlers=[hello_world],
    logging_config=logging_config,
    debug=True,
)

if __name__ == "__main__":
    with TestClient(app=app) as client:
        for _ in range(1_000):
            response = client.get("/")
