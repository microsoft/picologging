from _typeshed import StrPath, SupportsWrite
from collections.abc import Callable, Iterable, Mapping
from io import TextIOWrapper
from multiprocessing import Manager
from string import Template
from types import TracebackType
from typing import Any, Generic, Pattern, TextIO, TypeVar, Union, overload, Optional
from typing_extensions import Literal, TypeAlias

CRITICAL: int
FATAL: int
ERROR: int
WARNING: int
WARN: int
INFO: int
DEBUG: int
NOTSET: int

_SysExcInfoType: TypeAlias = Union[
    tuple[type[BaseException], BaseException, TracebackType | None],
    tuple[None, None, None],
]
_ExcInfoType: TypeAlias = None | bool | _SysExcInfoType | BaseException
_ArgsType: TypeAlias = tuple[object, ...] | Mapping[str, object]
_Level: TypeAlias = int  # Not | str like it is in logging
_FormatStyle: TypeAlias = Literal["%", "{", "$"]

class LogRecord:
    """
    A LogRecord instance represents an event being logged.

    LogRecord instances are created every time something is logged. They
    contain all the information pertinent to the event being logged. The
    main information passed in is in msg and args, which are combined
    using str(msg) % args to create the message field of the record. The
    record also includes information such as when the record was created,
    the source line where the logging call was made, and any exception
    information to be logged.
    """

    name: str
    levelno: int
    levelname: str
    msg: str
    args: Iterable[Any]
    pathname: str
    filename: str
    module: str
    lineno: int
    funcName: str
    created: float
    msecs: int
    relativeCreated: float
    thread: Optional[int]
    threadName: Optional[str]
    processName: Optional[str]
    process: Optional[int]
    exc_info: Any
    exc_text: Optional[str]
    stack_info: Optional[Any]
    message: str
    asctime: str
    def __init__(
        self,
        name: str,
        level: int,
        pathname: str,
        lineno: int,
        msg: str,
        args: Iterable[Any],
        exc_info: Any,
        func=None,
        sinfo=None,
        **kwargs,
    ): ...
    def getMessage(self) -> str:
        """
        Return the message for this LogRecord.

        Return the message for this LogRecord after merging any user-supplied
        arguments with the message.
        """
        ...

class Formatter:
    datefmt: str
    def __init__(
        self,
        fmt: str | None = ...,
        datefmt: str | None = ...,
        style: _FormatStyle = ...,
        validate: bool = ...,
        *,
        defaults: Mapping[str, Any] | None = ...,
    ) -> None: ...
    def format(self, record: LogRecord) -> str: ...
    def formatMessage(self, record: LogRecord) -> str: ...  # undocumented
    def formatStack(self, stack_info: str) -> str: ...
    def usesTime(self) -> bool: ...  # undocumented

_FilterType: TypeAlias = Filter | Callable[[LogRecord], int]

class Filterer:
    filters: list[Filter]
    def __init__(self) -> None: ...
    def addFilter(self, filter: _FilterType) -> None: ...
    def removeFilter(self, filter: _FilterType) -> None: ...
    def filter(self, record: LogRecord) -> bool: ...

class Handler(Filterer):
    level: int  # undocumented
    formatter: Formatter | None  # undocumented
    name: str | None  # undocumented
    def __init__(
        self, name: Optional[str] = None, level: Optional[int] = NOTSET
    ) -> None: ...
    def acquire(self) -> None: ...
    def release(self) -> None: ...
    def setLevel(self, level: int) -> None: ...
    def setFormatter(self, fmt: Formatter | None) -> None: ...
    def filter(self, record: LogRecord) -> bool: ...
    def handle(self, record: LogRecord) -> bool: ...
    def format(self, record: LogRecord) -> str: ...
    def emit(self, data: str) -> None: ...
    def flush(self) -> None: ...
    def close(self) -> None: ...
    def handleError(self, record: LogRecord) -> None: ...
    def get_name(self) -> str: ...
    def set_name(self, name: str) -> None: ...
    def createLock(self) -> None: ...

class Logger(Filterer):
    propagate: bool
    name: str
    level: int
    parent: Logger
    handlers: list[Handler]
    disabled: bool
    manager: Optional[Manager]
    def __init__(self, name: str, level: _Level = ...) -> None: ...
    def setLevel(self, level: _Level) -> None: ...
    def getEffectiveLevel(self) -> int: ...
    def debug(
        self,
        msg: object,
        *args: object,
        exc_info: _ExcInfoType = ...,
        stack_info: bool = ...,
        stacklevel: int = ...,
        extra: Mapping[str, object] | None = ...,
    ) -> None: ...
    def info(
        self,
        msg: object,
        *args: object,
        exc_info: _ExcInfoType = ...,
        stack_info: bool = ...,
        stacklevel: int = ...,
        extra: Mapping[str, object] | None = ...,
    ) -> None: ...
    def warning(
        self,
        msg: object,
        *args: object,
        exc_info: _ExcInfoType = ...,
        stack_info: bool = ...,
        stacklevel: int = ...,
        extra: Mapping[str, object] | None = ...,
    ) -> None: ...
    def warn(
        self,
        msg: object,
        *args: object,
        exc_info: _ExcInfoType = ...,
        stack_info: bool = ...,
        stacklevel: int = ...,
        extra: Mapping[str, object] | None = ...,
    ) -> None: ...
    def error(
        self,
        msg: object,
        *args: object,
        exc_info: _ExcInfoType = ...,
        stack_info: bool = ...,
        stacklevel: int = ...,
        extra: Mapping[str, object] | None = ...,
    ) -> None: ...
    def exception(
        self,
        msg: object,
        *args: object,
        exc_info: _ExcInfoType = ...,
        stack_info: bool = ...,
        stacklevel: int = ...,
        extra: Mapping[str, object] | None = ...,
    ) -> None: ...
    def critical(
        self,
        msg: object,
        *args: object,
        exc_info: _ExcInfoType = ...,
        stack_info: bool = ...,
        stacklevel: int = ...,
        extra: Mapping[str, object] | None = ...,
    ) -> None: ...
    def log(
        self,
        level: int,
        msg: object,
        *args: object,
        exc_info: _ExcInfoType = ...,
        stack_info: bool = ...,
        stacklevel: int = ...,
        extra: Mapping[str, object] | None = ...,
    ) -> None: ...
    fatal = critical
    def filter(self, record: LogRecord) -> bool: ...
    def addHandler(self, hdlr: Handler) -> None: ...
    def removeHandler(self, hdlr: Handler) -> None: ...
    def handle(self, record: LogRecord) -> None: ...

class Filter:
    name: str  # undocumented
    nlen: int  # undocumented
    def __init__(self, name: str = ...) -> None: ...
    def filter(self, record: LogRecord) -> bool: ...

def getLogger(name: str | None = ...) -> Logger: ...
def debug(
    msg: object,
    *args: object,
    exc_info: _ExcInfoType = ...,
    stack_info: bool = ...,
    stacklevel: int = ...,
    extra: Mapping[str, object] | None = ...,
) -> None: ...
def info(
    msg: object,
    *args: object,
    exc_info: _ExcInfoType = ...,
    stack_info: bool = ...,
    stacklevel: int = ...,
    extra: Mapping[str, object] | None = ...,
) -> None: ...
def warning(
    msg: object,
    *args: object,
    exc_info: _ExcInfoType = ...,
    stack_info: bool = ...,
    stacklevel: int = ...,
    extra: Mapping[str, object] | None = ...,
) -> None: ...
def warn(
    msg: object,
    *args: object,
    exc_info: _ExcInfoType = ...,
    stack_info: bool = ...,
    stacklevel: int = ...,
    extra: Mapping[str, object] | None = ...,
) -> None: ...
def error(
    msg: object,
    *args: object,
    exc_info: _ExcInfoType = ...,
    stack_info: bool = ...,
    stacklevel: int = ...,
    extra: Mapping[str, object] | None = ...,
) -> None: ...
def critical(
    msg: object,
    *args: object,
    exc_info: _ExcInfoType = ...,
    stack_info: bool = ...,
    stacklevel: int = ...,
    extra: Mapping[str, object] | None = ...,
) -> None: ...
def exception(
    msg: object,
    *args: object,
    exc_info: _ExcInfoType = ...,
    stack_info: bool = ...,
    stacklevel: int = ...,
    extra: Mapping[str, object] | None = ...,
) -> None: ...
def log(
    level: int,
    msg: object,
    *args: object,
    exc_info: _ExcInfoType = ...,
    stack_info: bool = ...,
    stacklevel: int = ...,
    extra: Mapping[str, object] | None = ...,
) -> None: ...
def basicConfig(
    *,
    filename: StrPath | None = ...,
    filemode: str = ...,
    format: str = ...,
    datefmt: str | None = ...,
    style: _FormatStyle = ...,
    level: _Level | None = ...,
    stream: SupportsWrite[str] | None = ...,
    handlers: Iterable[Handler] | None = ...,
    force: bool | None = ...,
    encoding: str | None = ...,
    errors: str | None = ...,
) -> None: ...

lastResort: StreamHandler[Any] | None

_StreamT = TypeVar("_StreamT", bound=SupportsWrite[str])

class StreamHandler(Handler, Generic[_StreamT]):
    stream: _StreamT  # undocumented
    @overload
    def __init__(self: StreamHandler[TextIO], stream: None = ...) -> None: ...
    @overload
    def __init__(self: StreamHandler[_StreamT], stream: _StreamT) -> None: ...
    def setStream(self, stream: _StreamT) -> _StreamT | None: ...

class FileHandler(StreamHandler[TextIOWrapper]):
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

class NullHandler(Handler): ...

root: Logger

class PercentStyle:
    def __init__(
        self, fmt: str, *, defaults: Mapping[str, Any] | None = ...
    ) -> None: ...
    def usesTime(self) -> bool: ...
    def validate(self) -> None: ...
    def format(self, record: Any) -> str: ...

class StrFormatStyle(PercentStyle):  # undocumented
    fmt_spec: Pattern[str]
    field_spec: Pattern[str]

class StringTemplateStyle(PercentStyle):  # undocumented
    _tpl: Template

_STYLES: dict[str, tuple[PercentStyle, str]]

BASIC_FORMAT: str

def getLevelName(level: _Level) -> Any: ...
def makeLogRecord(dict: Mapping[str, object]) -> LogRecord: ...
