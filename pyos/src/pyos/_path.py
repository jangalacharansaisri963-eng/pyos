from __future__ import annotations


_SEP = "/"
_ALTSEP = None
_CURDIR = "."
_PARDIR = ".."
_EXTSEP = "."


def _check_arg_types(*paths):
    kind = None

    for path in paths:
        path = fspath(path)

        if isinstance(path, str):
            current = str
        elif isinstance(path, bytes):
            current = bytes
        else:
            raise TypeError(
                "expected str or bytes path, got "
                + type(path).__name__
            )

        if kind is None:
            kind = current
        elif kind is not current:
            raise TypeError(
                "Can't mix strings and bytes in path components"
            )

    return kind


def fspath(path):
    if isinstance(path, (str, bytes)):
        return path

    method = getattr(path, "__fspath__", None)

    if method is None:
        raise TypeError(
            "expected str, bytes or path-like object"
        )

    value = method()

    if not isinstance(value, (str, bytes)):
        raise TypeError(
            "__fspath__() must return str or bytes"
        )

    return value


_fspath = fspath


def _empty(path):
    return b"" if isinstance(path, bytes) else ""


def _separator(path):
    return b"/" if isinstance(path, bytes) else "/"


def _curdir(path):
    return b"." if isinstance(path, bytes) else "."


def _pardir(path):
    return b".." if isinstance(path, bytes) else ".."


def join(path, *paths):
    path = fspath(path)
    result = path
    separator = _separator(path)
    empty = _empty(path)

    for component in paths:
        component = fspath(component)

        if type(component) is not type(result):
            raise TypeError(
                "Can't mix strings and bytes in path components"
            )

        if not component:
            continue

        if component.startswith(separator):
            result = component
        elif not result:
            result = component
        elif result.endswith(separator):
            result += component
        else:
            result += separator + component

    return result


def split(path):
    path = fspath(path)
    separator = _separator(path)

    stripped = path.rstrip(separator)

    if not stripped:
        if path.startswith(separator):
            return separator, _empty(path)

        return _empty(path), _empty(path)

    index = stripped.rfind(separator)

    if index < 0:
        return _empty(path), stripped

    head = stripped[:index]
    tail = stripped[index + 1:]

    while head.endswith(separator):
        head = head[:-1]

    if not head and path.startswith(separator):
        head = separator

    return head, tail


def splitdrive(path):
    path = fspath(path)
    return _empty(path), path


def splitroot(path):
    path = fspath(path)
    separator = _separator(path)
    empty = _empty(path)

    if not path.startswith(separator):
        return empty, empty, path

    index = 0

    while index < len(path) and path[index:index + 1] == separator:
        index += 1

    return empty, separator, path[index:]


def basename(path):
    return split(path)[1]


def dirname(path):
    return split(path)[0]


def isabs(path):
    path = fspath(path)
    return path.startswith(_separator(path))


def normpath(path):
    path = fspath(path)

    separator = _separator(path)
    curdir = _curdir(path)
    pardir = _pardir(path)

    if not path:
        return curdir

    absolute = path.startswith(separator)
    parts = path.split(separator)
    result = []

    for part in parts:
        if not part or part == curdir:
            continue

        if part == pardir:
            if result and result[-1] != pardir:
                result.pop()
            elif not absolute:
                result.append(pardir)
            continue

        result.append(part)

    value = separator.join(result)

    if absolute:
        value = separator + value

    if not value:
        return separator if absolute else curdir

    return value


def abspath(path):
    path = fspath(path)

    if isabs(path):
        return normpath(path)

    cwd = _getcwd_for_type(path)

    return normpath(join(cwd, path))


def relpath(path, start=None):
    path = fspath(path)

    if start is None:
        start = _getcwd_for_type(path)
    else:
        start = fspath(start)

    if type(path) is not type(start):
        raise TypeError(
            "path and start must be the same type"
        )

    path = abspath(path)
    start = abspath(start)

    separator = _separator(path)
    pardir = _pardir(path)
    curdir = _curdir(path)
    empty = _empty(path)

    if path == start:
        return curdir

    path_parts = normpath(path).split(separator)
    start_parts = normpath(start).split(separator)

    if path_parts and path_parts[0] == empty:
        path_parts = path_parts[1:]

    if start_parts and start_parts[0] == empty:
        start_parts = start_parts[1:]

    common = 0

    while (
        common < len(path_parts)
        and common < len(start_parts)
        and path_parts[common] == start_parts[common]
    ):
        common += 1

    result = []

    for _ in range(len(start_parts) - common):
        result.append(pardir)

    result.extend(path_parts[common:])

    if not result:
        return curdir

    return separator.join(result)


def commonprefix(paths):
    if not paths:
        return ""

    paths = [fspath(path) for path in paths]
    first = paths[0]

    for path in paths[1:]:
        if type(path) is not type(first):
            raise TypeError(
                "Can't mix strings and bytes in path components"
            )

    length = len(first)

    for path in paths[1:]:
        limit = min(length, len(path))
        index = 0

        while index < limit and first[index] == path[index]:
            index += 1

        length = index

    return first[:length]


def commonpath(paths):
    if not paths:
        raise ValueError(
            "commonpath() arg is an empty sequence"
        )

    paths = [fspath(path) for path in paths]
    kind = type(paths[0])

    for path in paths:
        if type(path) is not kind:
            raise TypeError(
                "Can't mix strings and bytes in path components"
            )

    normalized = [normpath(path) for path in paths]
    absolute = isabs(normalized[0])

    for path in normalized:
        if isabs(path) != absolute:
            raise ValueError(
                "Can't mix absolute and relative paths"
            )

    separator = _separator(normalized[0])
    empty = _empty(normalized[0])
    curdir = _curdir(normalized[0])

    split_paths = []

    for path in normalized:
        parts = path.split(separator)

        if absolute and parts and parts[0] == empty:
            parts = parts[1:]

        split_paths.append(parts)

    common = []

    for index in range(
        min(len(parts) for parts in split_paths)
    ):
        value = split_paths[0][index]

        if any(
            parts[index] != value
            for parts in split_paths[1:]
        ):
            break

        common.append(value)

    result = separator.join(common)

    if absolute:
        return separator + result

    return result or curdir


def expanduser(path):
    path = fspath(path)
    separator = _separator(path)

    if isinstance(path, bytes):
        if path != b"~" and not path.startswith(b"~/"):
            return path

        home = _getenvb(b"HOME")

        if home is None:
            return path

        if path == b"~":
            return home

        return home + path[1:]

    if path != "~" and not path.startswith("~/"):
        return path

    home = _getenv("HOME")

    if home is None:
        return path

    if path == "~":
        return home

    return home + path[1:]


def expandvars(path):
    path = fspath(path)

    if isinstance(path, bytes):
        return _expandvars_bytes(path)

    return _expandvars_string(path)


def _expandvars_string(path):
    result = []
    index = 0

    while index < len(path):
        if path[index] != "$":
            result.append(path[index])
            index += 1
            continue

        if index + 1 >= len(path):
            result.append("$")
            break

        if path[index + 1] == "{":
            end = path.find("}", index + 2)

            if end < 0:
                result.append("$")
                index += 1
                continue

            name = path[index + 2:end]

            if name:
                value = _getenv(name)

                if value is None:
                    result.append(path[index:end + 1])
                else:
                    result.append(value)

                index = end + 1
                continue

        end = index + 1

        while end < len(path):
            char = path[end]

            if not (
                char.isalnum()
                or char == "_"
            ):
                break

            end += 1

        name = path[index + 1:end]

        if not name:
            result.append("$")
            index += 1
            continue

        value = _getenv(name)

        if value is None:
            result.append("$" + name)
        else:
            result.append(value)

        index = end

    return "".join(result)


def _expandvars_bytes(path):
    result = []
    index = 0

    while index < len(path):
        char = path[index:index + 1]

        if char != b"$":
            result.append(char)
            index += 1
            continue

        if index + 1 >= len(path):
            result.append(b"$")
            break

        if path[index + 1:index + 2] == b"{":
            end = path.find(b"}", index + 2)

            if end < 0:
                result.append(b"$")
                index += 1
                continue

            name = path[index + 2:end]
            value = _getenvb(name)

            if value is None:
                result.append(path[index:end + 1])
            else:
                result.append(value)

            index = end + 1
            continue

        end = index + 1

        while end < len(path):
            char = path[end:end + 1]

            if not (
                b"a" <= char <= b"z"
                or b"A" <= char <= b"Z"
                or b"0" <= char <= b"9"
                or char == b"_"
            ):
                break

            end += 1

        name = path[index + 1:end]

        if not name:
            result.append(b"$")
            index += 1
            continue

        value = _getenvb(name)

        if value is None:
            result.append(b"$" + name)
        else:
            result.append(value)

        index = end

    return b"".join(result)


def normcase(path):
    return fspath(path)


def realpath(path):
    return abspath(path)


def lexists(path):
    checker = _get_filesystem()

    if checker is None:
        return False

    try:
        checker.lstat(path)
        return True
    except OSError:
        return False


def exists(path):
    checker = _get_filesystem()

    if checker is None:
        return False

    try:
        checker.stat(path)
        return True
    except OSError:
        return False


def isfile(path):
    checker = _get_filesystem()

    if checker is None:
        return False

    try:
        info = checker.stat(path)
    except OSError:
        return False

    return _is_regular(info)


def isdir(path):
    checker = _get_filesystem()

    if checker is None:
        return False

    try:
        info = checker.stat(path)
    except OSError:
        return False

    return _is_directory(info)


def islink(path):
    checker = _get_filesystem()

    if checker is None:
        return False

    try:
        info = checker.lstat(path)
    except OSError:
        return False

    return _is_link(info)


def samefile(path1, path2):
    checker = _get_filesystem()

    if checker is not None:
        return checker.samefile(path1, path2)

    return normpath(abspath(path1)) == normpath(abspath(path2))


def sameopenfile(fp1, fp2):
    first = _extract_fd(fp1)
    second = _extract_fd(fp2)

    return first == second


def getatime(path):
    return _stat_value(path, "st_atime")


def getmtime(path):
    return _stat_value(path, "st_mtime")


def getctime(path):
    return _stat_value(path, "st_ctime")


def getsize(path):
    return _stat_value(path, "st_size")


def _extract_fd(value):
    if isinstance(value, int):
        return value

    method = getattr(value, "fileno", None)

    if method is None:
        raise TypeError(
            "expected integer file descriptor or file object"
        )

    return method()


def _stat_value(path, name):
    checker = _get_filesystem()

    if checker is None:
        raise RuntimeError(
            "filesystem backend has not been initialized"
        )

    info = checker.stat(path)

    if hasattr(info, name):
        return getattr(info, name)

    if isinstance(info, dict) and name in info:
        return info[name]

    raise AttributeError(
        "stat result has no attribute " + name
    )


def _stat_mode(info):
    return _stat_value_from_info(info, "st_mode")


def _stat_value_from_info(info, name):
    if hasattr(info, name):
        return getattr(info, name)

    if isinstance(info, dict) and name in info:
        return info[name]

    raise AttributeError(
        "stat result has no attribute " + name
    )


def _is_regular(info):
    method = getattr(info, "is_file", None)

    if method is not None:
        return bool(method())

    if isinstance(info, dict):
        value = info.get("is_file")

        if value is not None:
            return bool(value)

    return (
        _stat_mode(info) & 0o170000
    ) == 0o100000


def _is_directory(info):
    method = getattr(info, "is_dir", None)

    if method is not None:
        return bool(method())

    if isinstance(info, dict):
        value = info.get("is_dir")

        if value is not None:
            return bool(value)

    return (
        _stat_mode(info) & 0o170000
    ) == 0o040000


def _is_link(info):
    method = getattr(info, "is_symlink", None)

    if method is not None:
        return bool(method())

    if isinstance(info, dict):
        value = info.get("is_symlink")

        if value is not None:
            return bool(value)

    return (
        _stat_mode(info) & 0o170000
    ) == 0o120000


def _get_filesystem():
    return globals().get("_filesystem")


def _getcwd_for_type(path):
    cwd = globals().get("_cwd")

    if cwd is None:
        if isinstance(path, bytes):
            return b"/"

        return "/"

    if type(cwd) is not type(path):
        raise TypeError(
            "current working directory and path "
            "must be the same type"
        )

    return cwd


def _getenv(name):
    environ = globals().get("_environ")

    if environ is None:
        return None

    return environ.get(name)


def _getenvb(name):
    environ = globals().get("_environb")

    if environ is None:
        return None

    return environ.get(name)


def _set_backend(
    cwd=None,
    environ=None,
    environb=None,
    filesystem=None,
    stat=None,
):
    if cwd is not None:
        globals()["_cwd"] = cwd

    if environ is not None:
        globals()["_environ"] = environ

    if environb is not None:
        globals()["_environb"] = environb

    if filesystem is not None:
        globals()["_filesystem"] = filesystem

    if stat is not None:
        globals()["_stat"] = stat


def _clear_backend():
    for name in (
        "_cwd",
        "_environ",
        "_environb",
        "_filesystem",
        "_stat",
    ):
        globals().pop(name, None)


__all__ = [
    "fspath",
    "_fspath",
    "join",
    "split",
    "splitdrive",
    "splitroot",
    "basename",
    "dirname",
    "isabs",
    "normpath",
    "abspath",
    "relpath",
    "commonprefix",
    "commonpath",
    "expanduser",
    "expandvars",
    "normcase",
    "realpath",
    "exists",
    "lexists",
    "isfile",
    "isdir",
    "islink",
    "samefile",
    "sameopenfile",
    "getatime",
    "getmtime",
    "getctime",
    "getsize",
    ]
