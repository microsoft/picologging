import picologging
from picologging import LogRecord


def log():
    for _ in range(100_000):
        record = LogRecord(
            "hello", picologging.WARNING, __file__, 123, "bork %s", None, None
        )
        assert record.message is None
        assert record.getMessage() == "bork %s"
        assert record.message == "bork %s"
        assert "name" in record.__dict__


if __name__ == "__main__":
    log()
