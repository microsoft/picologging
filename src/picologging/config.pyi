import sys
from collections.abc import Sequence
from typing import Any

from . import _Level

if sys.version_info >= (3, 8):
    from typing import Literal, TypedDict
else:
    from typing_extensions import Literal, TypedDict

class _RootLoggerConfiguration(TypedDict, total=False):
    level: _Level
    filters: Sequence[str]
    handlers: Sequence[str]

class _LoggerConfiguration(_RootLoggerConfiguration, TypedDict, total=False):
    propagate: bool

class _OptionalDictConfigArgs(TypedDict, total=False):
    # these two can have custom factories (key: `()`) which can have extra keys
    formatters: dict[str, dict[str, Any]]
    filters: dict[str, dict[str, Any]]
    # type checkers would warn about extra keys if this was a TypedDict
    handlers: dict[str, dict[str, Any]]
    loggers: dict[str, _LoggerConfiguration]
    root: _RootLoggerConfiguration | None
    incremental: bool
    disable_existing_loggers: bool

class _DictConfigArgs(_OptionalDictConfigArgs, TypedDict):
    version: Literal[1]

# Accept dict[str, Any] to avoid false positives if called with a dict
# type, since dict types are not compatible with TypedDicts.
#
# Also accept a TypedDict type, to allow callers to use TypedDict
# types, and for somewhat stricter type checking of dict literals.
def dictConfig(config: _DictConfigArgs | dict[str, Any]) -> None: ...
def valid_ident(s: str) -> Literal[True]: ...  # undocumented
