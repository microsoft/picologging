from typing import Callable
from _typeshed import StrPath

from picologging import FileHandler

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
