"""pyos: a native POSIX operating-system interface for Python."""

__version__ = "0.1.0"

name = "posix"

try:
    from . import _native as _native_mod
except Exception:  # pragma: no cover
    _native_mod = None

from . import _native_posix
from . import _posix
from . import path

if _native_mod is not None:
    _native_posix.initialize(_native_mod)

__all__ = [
    "__version__",
    "name",
    "path",
    "_native_posix",
    "_posix",
]
