from datetime import datetime
from typing import Any, Callable, Pattern
from _typeshed import StrPath
from queue import Queue, SimpleQueue
from socket import socket

from picologging import Handler, FileHandler, LogRecord

class WatchedFileHandler(FileHandler):
    baseFilename: str  # undocumented
    mode: str  # undocumented
    encoding: str | None  # undocumented
    delay: bool  # undocumented
    errors: str | None  # undocumented
    def __init__(
        self,
        filename: StrPath,
        mode: str = ...,
        encoding: str | None = ...,
        delay: bool = ...,
        errors: str | None = ...,
    ) -> None: ...

class BaseRotatingHandler(FileHandler):
    namer: Callable[[str], str] | None
    rotator: Callable[[str, str], None] | None
    def __init__(
        self,
        filename: StrPath,
        mode: str,
        encoding: str | None = ...,
        delay: bool = ...,
        errors: str | None = ...,
    ) -> None: ...
    def rotation_filename(self, default_name: str) -> str: ...
    def rotate(self, source: str, dest: str) -> None: ...

class RotatingFileHandler(BaseRotatingHandler):
    maxBytes: str  # undocumented
    backupCount: int  # undocumented
    def __init__(
        self,
        filename: StrPath,
        mode: str = ...,
        maxBytes: int = ...,
        backupCount: int = ...,
        encoding: str | None = ...,
        delay: bool = ...,
        errors: str | None = ...,
    ) -> None: ...
    def doRollover(self) -> None: ...
    def shouldRollover(self, record: LogRecord) -> int: ...  # undocumented

class TimedRotatingFileHandler(BaseRotatingHandler):
    when: str  # undocumented
    backupCount: int  # undocumented
    utc: bool  # undocumented
    atTime: datetime.time | None  # undocumented
    interval: int  # undocumented
    suffix: str  # undocumented
    dayOfWeek: int  # undocumented
    rolloverAt: int  # undocumented
    extMatch: Pattern[str]  # undocumented
    def __init__(
        self,
        filename: StrPath,
        when: str = ...,
        interval: int = ...,
        backupCount: int = ...,
        encoding: str | None = ...,
        delay: bool = ...,
        utc: bool = ...,
        atTime: datetime.time | None = ...,
        errors: str | None = ...,
    ) -> None: ...
    def doRollover(self) -> None: ...
    def shouldRollover(self, record: LogRecord) -> int: ...  # undocumented
    def computeRollover(self, currentTime: int) -> int: ...  # undocumented
    def getFilesToDelete(self) -> list[str]: ...  # undocumented

class QueueHandler(Handler):
    queue: SimpleQueue[Any] | Queue[Any]  # undocumented
    def __init__(self, queue: SimpleQueue[Any] | Queue[Any]) -> None: ...

class QueueListener:
    handlers: tuple[Handler, ...]  # undocumented
    respect_handler_level: bool  # undocumented
    queue: SimpleQueue[Any] | Queue[Any]  # undocumented
    def __init__(
        self,
        queue: SimpleQueue[Any] | Queue[Any],
        *handlers: Handler,
        respect_handler_level: bool = ...
    ) -> None: ...
    def dequeue(self, block: bool) -> LogRecord: ...
    def prepare(self, record: LogRecord) -> Any: ...
    def start(self) -> None: ...
    def stop(self) -> None: ...
    def enqueue_sentinel(self) -> None: ...
    def handle(self, record: LogRecord) -> None: ...

class BufferingHandler(Handler):
    capacity: int  # undocumented
    buffer: list[LogRecord]  # undocumented
    def __init__(self, capacity: int) -> None: ...

class MemoryHandler(BufferingHandler):
    flushLevel: int  # undocumented
    target: Handler | None  # undocumented
    flushOnClose: bool  # undocumented
    def __init__(
        self,
        capacity: int,
        flushLevel: int = ...,
        target: Handler | None = ...,
        flushOnClose: bool = ...,
    ) -> None: ...
    def setTarget(self, target: Handler | None) -> None: ...

class SocketHandler(Handler):
    host: str  # undocumented
    port: int | None  # undocumented
    address: tuple[str, int] | str  # undocumented
    sock: socket | None  # undocumented
    closeOnError: bool  # undocumented
    retryTime: float | None  # undocumented
    retryStart: float  # undocumented
    retryFactor: float  # undocumented
    retryMax: float  # undocumented
    def __init__(self, host: str, port: int | None) -> None: ...
    def makeSocket(self, timeout: float = ...) -> socket: ...  # timeout is undocumented
    def makePickle(self, record: LogRecord) -> bytes: ...
    def send(self, s: bytes) -> None: ...
    def createSocket(self) -> None: ...
