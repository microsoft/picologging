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
