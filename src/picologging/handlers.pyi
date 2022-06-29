from typing import Callable
from _typeshed import StrPath

from picologging import FileHandler, LogRecord


class WatchedFileHandler(FileHandler):
    baseFilename: str  # undocumented
    mode: str  # undocumented
    encoding: str | None  # undocumented
    delay: bool  # undocumented
    errors: str | None  # undocumented
    def __init__(
        self, filename: StrPath, mode: str = ..., encoding: str | None = ..., delay: bool = ..., errors: str | None = ...
    ) -> None: ...


class BaseRotatingHandler(FileHandler):
    namer: Callable[[str], str] | None
    rotator: Callable[[str, str], None] | None
    def __init__(
        self, filename: StrPath, mode: str, encoding: str | None = ..., delay: bool = ..., errors: str | None = ...
    ) -> None: ...

    def rotation_filename(self, default_name: str) -> str: ...
    def rotate(self, source: str, dest: str) -> None: ...


class RotatingFileHandler(BaseRotatingHandler):
    maxBytes: str  # undocumented
    backupCount: int  # undocumented
    def __init__(
        self, filename: StrPath, mode: str = ..., maxBytes: int = ..., backupCount: int = ...,
        encoding: str | None = ..., delay: bool = ..., errors: str | None = ...,
    ) -> None: ...

    def doRollover(self) -> None: ...
    def shouldRollover(self, record: LogRecord) -> int: ...  # undocumented
