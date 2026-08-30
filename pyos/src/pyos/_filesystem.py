from __future__ import annotations


class FileSystemError(OSError):
    pass


class BackendNotInitializedError(RuntimeError):
    pass


class BackendCapabilityError(NotImplementedError):
    pass


class FileSystemBackend:
    name = "abstract"
    platform = "unknown"

    def getcwd(self):
        raise NotImplementedError

    def chdir(self, path):
        raise NotImplementedError

    def listdir(self, path="."):
        raise NotImplementedError

    def scandir(self, path="."):
        raise NotImplementedError

    def mkdir(self, path, mode=0o777, *, dir_fd=None):
        raise NotImplementedError

    def makedirs(self, path, mode=0o777, exist_ok=False):
        path = _normalize_path(path)

        if not path:
            raise FileNotFoundError(path)

        if self.isdir(path):
            if exist_ok:
                return None

            raise FileExistsError(path)

        parent = _parent_path(path)

        if parent and parent != path:
            if not self.exists(parent):
                self.makedirs(
                    parent,
                    mode,
                    exist_ok=True,
                )

        try:
            self.mkdir(path, mode)
        except FileExistsError:
            if not exist_ok or not self.isdir(path):
                raise

        return None

    def rmdir(self, path, *, dir_fd=None):
        raise NotImplementedError

    def removedirs(self, path):
        path = _normalize_path(path)

        self.rmdir(path)

        current = _parent_path(path)

        while current and current != ".":
            try:
                self.rmdir(current)
            except OSError:
                break

            current = _parent_path(current)

        return None

    def remove(self, path, *, dir_fd=None):
        raise NotImplementedError

    def unlink(self, path, *, dir_fd=None):
        return self.remove(
            path,
            dir_fd=dir_fd,
        )

    def rename(
        self,
        old,
        new,
        *,
        src_dir_fd=None,
        dst_dir_fd=None,
    ):
        raise NotImplementedError

    def replace(
        self,
        old,
        new,
        *,
        src_dir_fd=None,
        dst_dir_fd=None,
    ):
        raise NotImplementedError

    def stat(
        self,
        path,
        *,
        dir_fd=None,
        follow_symlinks=True,
    ):
        raise NotImplementedError

    def lstat(self, path, *, dir_fd=None):
        return self.stat(
            path,
            dir_fd=dir_fd,
            follow_symlinks=False,
        )

    def fstat(self, fd):
        raise NotImplementedError

    def chmod(
        self,
        path,
        mode,
        *,
        dir_fd=None,
        follow_symlinks=True,
    ):
        raise NotImplementedError

    def access(
        self,
        path,
        mode,
        *,
        dir_fd=None,
        effective_ids=False,
        follow_symlinks=True,
    ):
        raise NotImplementedError

    def readlink(self, path, *, dir_fd=None):
        raise NotImplementedError

    def symlink(
        self,
        src,
        dst,
        target_is_directory=False,
        *,
        dir_fd=None,
    ):
        raise NotImplementedError

    def link(
        self,
        src,
        dst,
        *,
        src_dir_fd=None,
        dst_dir_fd=None,
        follow_symlinks=True,
    ):
        raise NotImplementedError

    def truncate(self, path, length):
        raise NotImplementedError

    def open(self, path, flags, mode=0o666):
        raise NotImplementedError

    def close(self, fd):
        raise NotImplementedError

    def read(self, fd, size=-1):
        raise NotImplementedError

    def write(self, fd, data):
        raise NotImplementedError

    def seek(self, fd, offset, whence=0):
        raise NotImplementedError

    def fsync(self, fd):
        raise NotImplementedError

    def dup(self, fd):
        raise NotImplementedError

    def dup2(self, fd, target_fd):
        raise NotImplementedError

    def pipe(self):
        raise NotImplementedError

    def isatty(self, fd):
        raise NotImplementedError

    def disk_usage(self, path="."):
        raise NotImplementedError

    def realpath(self, path):
        raise NotImplementedError

    def getcwd_bytes(self):
        value = self.getcwd()

        if isinstance(value, bytes):
            return value

        return value.encode(
            "utf-8",
            "surrogateescape",
        )

    def exists(self, path):
        try:
            self.stat(path)
            return True
        except (OSError, ValueError):
            return False

    def lexists(self, path):
        try:
            self.lstat(path)
            return True
        except (OSError, ValueError):
            return False

    def isfile(self, path):
        try:
            info = self.stat(path)
        except (OSError, ValueError):
            return False

        return _stat_is_regular(info)

    def isdir(self, path):
        try:
            info = self.stat(path)
        except (OSError, ValueError):
            return False

        return _stat_is_directory(info)

    def islink(self, path):
        try:
            info = self.lstat(path)
        except (OSError, ValueError):
            return False

        return _stat_is_link(info)

    def getsize(self, path):
        return _stat_value(
            self.stat(path),
            "st_size",
        )

    def getatime(self, path):
        return _stat_value(
            self.stat(path),
            "st_atime",
        )

    def getmtime(self, path):
        return _stat_value(
            self.stat(path),
            "st_mtime",
        )

    def getctime(self, path):
        return _stat_value(
            self.stat(path),
            "st_ctime",
        )

    def samefile(self, path1, path2):
        first = self.stat(path1)
        second = self.stat(path2)

        return (
            _stat_value(first, "st_dev")
            == _stat_value(second, "st_dev")
            and
            _stat_value(first, "st_ino")
            == _stat_value(second, "st_ino")
        )

    def walk(
        self,
        top,
        *,
        topdown=True,
        onerror=None,
        followlinks=False,
    ):
        top = _normalize_path(top)

        try:
            names = self.listdir(top)
        except OSError as exc:
            if onerror is not None:
                onerror(exc)
            return

        directories = []
        files = []

        for name in names:
            full_path = _join_path(
                top,
                name,
            )

            try:
                if self.isdir(full_path):
                    if (
                        followlinks
                        or not self.islink(full_path)
                    ):
                        directories.append(name)
                    else:
                        files.append(name)
                else:
                    files.append(name)
            except OSError as exc:
                if onerror is not None:
                    onerror(exc)

        if topdown:
            yield top, directories, files

        for directory in directories:
            child = _join_path(
                top,
                directory,
            )

            yield from self.walk(
                child,
                topdown=topdown,
                onerror=onerror,
                followlinks=followlinks,
            )

        if not topdown:
            yield top, directories, files

    def supports(self, capability):
        method = getattr(
            self,
            capability,
            None,
        )

        if method is None:
            return False

        implementation = getattr(
            method,
            "__func__",
            method,
        )

        base = getattr(
            FileSystemBackend,
            capability,
            None,
        )

        return implementation is not base

    def capabilities(self):
        names = (
            "open",
            "close",
            "read",
            "write",
            "seek",
            "fsync",
            "dup",
            "dup2",
            "pipe",
            "isatty",
            "stat",
            "fstat",
            "lstat",
            "chmod",
            "access",
            "readlink",
            "symlink",
            "link",
            "truncate",
            "disk_usage",
            "realpath",
        )

        return {
            name: self.supports(name)
            for name in names
        }


def _normalize_path(path):
    if isinstance(path, bytes):
        if not path:
            return b"."

        return path

    if isinstance(path, str):
        if not path:
            return "."

        return path

    method = getattr(
        path,
        "__fspath__",
        None,
    )

    if method is None:
        raise TypeError(
            "path must be str, bytes, or path-like"
        )

    result = method()

    if isinstance(result, (str, bytes)):
        if not result:
            return (
                b"."
                if isinstance(result, bytes)
                else "."
            )

        return result

    raise TypeError(
        "__fspath__ returned a non-path value"
    )


def _path_separator(path):
    if isinstance(path, bytes):
        return b"/"

    return "/"


def _join_path(first, second):
    first = _normalize_path(first)
    second = _normalize_path(second)

    separator = _path_separator(first)

    if isinstance(first, bytes) != isinstance(second, bytes):
        raise TypeError(
            "cannot mix str and bytes paths"
        )

    if not first:
        return second

    if not second:
        return first

    if second.startswith(separator):
        return second

    if first.endswith(separator):
        return first + second

    return first + separator + second


def _parent_path(path):
    path = _normalize_path(path)
    separator = _path_separator(path)

    if isinstance(path, bytes):
        dot = b"."
        root = b"/"
    else:
        dot = "."
        root = "/"

    position = path.rfind(separator)

    if position < 0:
        return dot

    if position == 0:
        return root

    return path[:position]


def _basename(path):
    path = _normalize_path(path)
    separator = _path_separator(path)

    position = path.rfind(separator)

    if position < 0:
        return path

    return path[position + 1:]


def _stat_value(info, name):
    if hasattr(info, name):
        return getattr(info, name)

    if isinstance(info, dict):
        if name in info:
            return info[name]

    raise AttributeError(
        "stat result has no attribute " + name
    )


def _stat_mode(info):
    return _stat_value(
        info,
        "st_mode",
    )


def _stat_type(info):
    if isinstance(info, dict):
        value = info.get("type")

        if value is not None:
            return value

    if hasattr(info, "type"):
        return info.type

    return None


def _stat_is_regular(info):
    if hasattr(info, "is_file"):
        return bool(info.is_file())

    if isinstance(info, dict):
        value = info.get("is_file")

        if value is not None:
            return bool(value)

    type_value = _stat_type(info)

    if type_value is not None:
        return type_value == 8

    return (
        _stat_mode(info) & 0o170000
    ) == 0o100000


def _stat_is_directory(info):
    if hasattr(info, "is_dir"):
        return bool(info.is_dir())

    if isinstance(info, dict):
        value = info.get("is_dir")

        if value is not None:
            return bool(value)

    type_value = _stat_type(info)

    if type_value is not None:
        return type_value == 4

    return (
        _stat_mode(info) & 0o170000
    ) == 0o040000


def _stat_is_link(info):
    if hasattr(info, "is_symlink"):
        return bool(info.is_symlink())

    if isinstance(info, dict):
        value = info.get("is_symlink")

        if value is not None:
            return bool(value)

    type_value = _stat_type(info)

    if type_value is not None:
        return type_value == 10

    return (
        _stat_mode(info) & 0o170000
    ) == 0o120000


_backend = None


def set_backend(backend):
    global _backend

    if backend is not None:
        required = (
            "getcwd",
            "chdir",
            "listdir",
            "stat",
        )

        missing = []

        for name in required:
            method = getattr(
                backend,
                name,
                None,
            )

            if not callable(method):
                missing.append(name)

        if missing:
            raise TypeError(
                "invalid filesystem backend; "
                "missing: "
                + ", ".join(missing)
            )

    _backend = backend


def get_backend():
    return _backend


def clear_backend():
    global _backend
    _backend = None


def require_backend():
    backend = _backend

    if backend is None:
        raise BackendNotInitializedError(
            "pyos filesystem backend has not "
            "been initialized"
        )

    return backend


def getcwd():
    return require_backend().getcwd()


def chdir(path):
    return require_backend().chdir(
        _normalize_path(path)
    )


def listdir(path="."):
    return require_backend().listdir(
        _normalize_path(path)
    )


def scandir(path="."):
    return require_backend().scandir(
        _normalize_path(path)
    )


def mkdir(
    path,
    mode=0o777,
    *,
    dir_fd=None,
):
    return require_backend().mkdir(
        _normalize_path(path),
        mode,
        dir_fd=dir_fd,
    )


def makedirs(
    path,
    mode=0o777,
    exist_ok=False,
):
    return require_backend().makedirs(
        _normalize_path(path),
        mode,
        exist_ok,
    )


def rmdir(path, *, dir_fd=None):
    return require_backend().rmdir(
        _normalize_path(path),
        dir_fd=dir_fd,
    )


def removedirs(path):
    return require_backend().removedirs(
        _normalize_path(path)
    )


def remove(path, *, dir_fd=None):
    return require_backend().remove(
        _normalize_path(path),
        dir_fd=dir_fd,
    )


def unlink(path, *, dir_fd=None):
    return require_backend().unlink(
        _normalize_path(path),
        dir_fd=dir_fd,
    )


def rename(
    old,
    new,
    *,
    src_dir_fd=None,
    dst_dir_fd=None,
):
    return require_backend().rename(
        _normalize_path(old),
        _normalize_path(new),
        src_dir_fd=src_dir_fd,
        dst_dir_fd=dst_dir_fd,
    )


def replace(
    old,
    new,
    *,
    src_dir_fd=None,
    dst_dir_fd=None,
):
    return require_backend().replace(
        _normalize_path(old),
        _normalize_path(new),
        src_dir_fd=src_dir_fd,
        dst_dir_fd=dst_dir_fd,
    )


def stat(
    path,
    *,
    dir_fd=None,
    follow_symlinks=True,
):
    return require_backend().stat(
        _normalize_path(path),
        dir_fd=dir_fd,
        follow_symlinks=follow_symlinks,
    )


def lstat(path, *, dir_fd=None):
    return require_backend().lstat(
        _normalize_path(path),
        dir_fd=dir_fd,
    )


def fstat(fd):
    return require_backend().fstat(fd)


def chmod(
    path,
    mode,
    *,
    dir_fd=None,
    follow_symlinks=True,
):
    return require_backend().chmod(
        _normalize_path(path),
        mode,
        dir_fd=dir_fd,
        follow_symlinks=follow_symlinks,
    )


def access(
    path,
    mode,
    *,
    dir_fd=None,
    effective_ids=False,
    follow_symlinks=True,
):
    return require_backend().access(
        _normalize_path(path),
        mode,
        dir_fd=dir_fd,
        effective_ids=effective_ids,
        follow_symlinks=follow_symlinks,
    )


def readlink(path, *, dir_fd=None):
    return require_backend().readlink(
        _normalize_path(path),
        dir_fd=dir_fd,
    )


def symlink(
    src,
    dst,
    target_is_directory=False,
    *,
    dir_fd=None,
):
    return require_backend().symlink(
        _normalize_path(src),
        _normalize_path(dst),
        target_is_directory,
        dir_fd=dir_fd,
    )


def link(
    src,
    dst,
    *,
    src_dir_fd=None,
    dst_dir_fd=None,
    follow_symlinks=True,
):
    return require_backend().link(
        _normalize_path(src),
        _normalize_path(dst),
        src_dir_fd=src_dir_fd,
        dst_dir_fd=dst_dir_fd,
        follow_symlinks=follow_symlinks,
    )


def truncate(path, length):
    return require_backend().truncate(
        _normalize_path(path),
        length,
    )


def open(path, flags, mode=0o666):
    return require_backend().open(
        _normalize_path(path),
        flags,
        mode,
    )


def close(fd):
    return require_backend().close(fd)


def read(fd, size=-1):
    return require_backend().read(
        fd,
        size,
    )


def write(fd, data):
    return require_backend().write(
        fd,
        data,
    )


def seek(fd, offset, whence=0):
    return require_backend().seek(
        fd,
        offset,
        whence,
    )


def fsync(fd):
    return require_backend().fsync(fd)


def dup(fd):
    return require_backend().dup(fd)


def dup2(fd, target_fd):
    return require_backend().dup2(
        fd,
        target_fd,
    )


def pipe():
    return require_backend().pipe()


def isatty(fd):
    return require_backend().isatty(fd)


def exists(path):
    return require_backend().exists(
        _normalize_path(path)
    )


def lexists(path):
    return require_backend().lexists(
        _normalize_path(path)
    )


def isfile(path):
    return require_backend().isfile(
        _normalize_path(path)
    )


def isdir(path):
    return require_backend().isdir(
        _normalize_path(path)
    )


def islink(path):
    return require_backend().islink(
        _normalize_path(path)
    )


def getsize(path):
    return require_backend().getsize(
        _normalize_path(path)
    )


def getatime(path):
    return require_backend().getatime(
        _normalize_path(path)
    )


def getmtime(path):
    return require_backend().getmtime(
        _normalize_path(path)
    )


def getctime(path):
    return require_backend().getctime(
        _normalize_path(path)
    )


def samefile(path1, path2):
    return require_backend().samefile(
        _normalize_path(path1),
        _normalize_path(path2),
    )


def walk(
    top,
    *,
    topdown=True,
    onerror=None,
    followlinks=False,
):
    return require_backend().walk(
        _normalize_path(top),
        topdown=topdown,
        onerror=onerror,
        followlinks=followlinks,
    )


def disk_usage(path="."):
    return require_backend().disk_usage(
        _normalize_path(path)
    )


def realpath(path):
    return require_backend().realpath(
        _normalize_path(path)
    )


def capabilities():
    return require_backend().capabilities()


__all__ = [
    "FileSystemError",
    "BackendNotInitializedError",
    "BackendCapabilityError",
    "FileSystemBackend",
    "set_backend",
    "get_backend",
    "clear_backend",
    "require_backend",
    "getcwd",
    "chdir",
    "listdir",
    "scandir",
    "mkdir",
    "makedirs",
    "rmdir",
    "removedirs",
    "remove",
    "unlink",
    "rename",
    "replace",
    "stat",
    "lstat",
    "fstat",
    "chmod",
    "access",
    "readlink",
    "symlink",
    "link",
    "truncate",
    "open",
    "close",
    "read",
    "write",
    "seek",
    "fsync",
    "dup",
    "dup2",
    "pipe",
    "isatty",
    "exists",
    "lexists",
    "isfile",
    "isdir",
    "islink",
    "getsize",
    "getatime",
    "getmtime",
    "getctime",
    "samefile",
    "walk",
    "disk_usage",
    "realpath",
    "capabilities",
    ]
