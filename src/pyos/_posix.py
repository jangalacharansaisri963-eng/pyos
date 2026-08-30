from __future__ import annotations


class POSIXBackend:
    name = "posix"
    platform = "POSIX"

    def __init__(self, native=None):
        self._native = native

    def set_native(self, native):
        self._native = native
        return self

    def get_native(self):
        return self._native

    def available(self):
        return self._native is not None

    def _require_native(self):
        native = self._native

        if native is None:
            raise RuntimeError(
                "POSIX native provider has not been initialized"
            )

        return native

    def _call(self, name, *args, **kwargs):
        native = self._require_native()
        function = getattr(native, name, None)

        if function is None or not callable(function):
            raise NotImplementedError(
                "POSIX native provider does not implement "
                + name
            )

        return function(*args, **kwargs)

    def getcwd(self):
        return self._call("getcwd")

    def chdir(self, path):
        return self._call("chdir", path)

    def fchdir(self, fd):
        return self._call("fchdir", fd)

    def listdir(self, path="."):
        return self._call("listdir", path)

    def scandir(self, path="."):
        return self._call("scandir", path)

    def mkdir(self, path, mode=0o777):
        return self._call("mkdir", path, mode)

    def mkdirat(self, dirfd, path, mode=0o777):
        return self._call(
            "mkdirat",
            dirfd,
            path,
            mode,
        )

    def rmdir(self, path):
        return self._call("rmdir", path)

    def makedirs(self, path, mode=0o777, exist_ok=False):
        return self._call(
            "makedirs",
            path,
            mode,
            exist_ok,
        )

    def removedirs(self, path):
        return self._call("removedirs", path)

    def open(self, path, flags, mode=0o666):
        return self._call(
            "open",
            path,
            flags,
            mode,
        )

    def openat(self, dirfd, path, flags, mode=0o666):
        return self._call(
            "openat",
            dirfd,
            path,
            flags,
            mode,
        )

    def close(self, fd):
        return self._call("close", fd)

    def read(self, fd, size):
        return self._call("read", fd, size)

    def write(self, fd, data):
        return self._call("write", fd, data)

    def pread(self, fd, size, offset):
        return self._call(
            "pread",
            fd,
            size,
            offset,
        )

    def pwrite(self, fd, data, offset):
        return self._call(
            "pwrite",
            fd,
            data,
            offset,
        )

    def lseek(self, fd, position, whence=0):
        return self._call(
            "lseek",
            fd,
            position,
            whence,
        )

    def fsync(self, fd):
        return self._call("fsync", fd)

    def fdatasync(self, fd):
        return self._call("fdatasync", fd)

    def ftruncate(self, fd, length):
        return self._call(
            "ftruncate",
            fd,
            length,
        )

    def truncate(self, path, length):
        return self._call(
            "truncate",
            path,
            length,
        )

    def stat(self, path):
        return self._call("stat", path)

    def lstat(self, path):
        return self._call("lstat", path)

    def fstat(self, fd):
        return self._call("fstat", fd)

    def fstatat(self, dirfd, path, flags=0):
        return self._call(
            "fstatat",
            dirfd,
            path,
            flags,
        )

    def chmod(self, path, mode):
        return self._call("chmod", path, mode)

    def fchmod(self, fd, mode):
        return self._call("fchmod", fd, mode)

    def chown(self, path, uid, gid):
        return self._call(
            "chown",
            path,
            uid,
            gid,
        )

    def fchown(self, fd, uid, gid):
        return self._call(
            "fchown",
            fd,
            uid,
            gid,
        )

    def lchown(self, path, uid, gid):
        return self._call(
            "lchown",
            path,
            uid,
            gid,
        )

    def unlink(self, path):
        return self._call("unlink", path)

    def unlinkat(self, dirfd, path, flags=0):
        return self._call(
            "unlinkat",
            dirfd,
            path,
            flags,
        )

    def remove(self, path):
        return self.unlink(path)

    def rename(self, old, new):
        return self._call("rename", old, new)

    def renameat(self, olddirfd, old, newdirfd, new):
        return self._call(
            "renameat",
            olddirfd,
            old,
            newdirfd,
            new,
        )

    def replace(self, old, new):
        function = getattr(
            self._require_native(),
            "replace",
            None,
        )

        if function is not None:
            return function(old, new)

        return self.rename(old, new)

    def access(self, path, mode):
        return self._call("access", path, mode)

    def faccessat(self, dirfd, path, mode, flags=0):
        return self._call(
            "faccessat",
            dirfd,
            path,
            mode,
            flags,
        )

    def readlink(self, path):
        return self._call("readlink", path)

    def readlinkat(self, dirfd, path):
        return self._call(
            "readlinkat",
            dirfd,
            path,
        )

    def symlink(self, src, dst, target_is_directory=False):
        function = getattr(
            self._require_native(),
            "symlink",
        )

        try:
            return function(
                src,
                dst,
                target_is_directory,
            )
        except TypeError:
            return function(src, dst)

    def symlinkat(self, src, dirfd, dst):
        return self._call(
            "symlinkat",
            src,
            dirfd,
            dst,
        )

    def link(self, src, dst):
        return self._call("link", src, dst)

    def linkat(self, olddirfd, old, newdirfd, new):
        return self._call(
            "linkat",
            olddirfd,
            old,
            newdirfd,
            new,
        )

    def dup(self, fd):
        return self._call("dup", fd)

    def dup2(self, oldfd, newfd):
        return self._call(
            "dup2",
            oldfd,
            newfd,
        )

    def dup3(self, oldfd, newfd, flags=0):
        return self._call(
            "dup3",
            oldfd,
            newfd,
            flags,
        )

    def pipe(self):
        return self._call("pipe")

    def pipe2(self, flags=0):
        return self._call("pipe2", flags)

    def fcntl(self, fd, command, argument=0):
        return self._call(
            "fcntl",
            fd,
            command,
            argument,
        )

    def ioctl(self, fd, request, argument=None):
        if argument is None:
            return self._call(
                "ioctl",
                fd,
                request,
            )

        return self._call(
            "ioctl",
            fd,
            request,
            argument,
        )

    def isatty(self, fd):
        return self._call("isatty", fd)

    def ttyname(self, fd):
        return self._call("ttyname", fd)

    def sync(self):
        return self._call("sync")

    def syncfs(self, fd):
        return self._call("syncfs", fd)

    def uname(self):
        return self._call("uname")

    def sysconf(self, name):
        return self._call("sysconf", name)

    def pathconf(self, path, name):
        return self._call(
            "pathconf",
            path,
            name,
        )

    def fpathconf(self, fd, name):
        return self._call(
            "fpathconf",
            fd,
            name,
        )

    def getloadavg(self):
        return self._call("getloadavg")

    def getpagesize(self):
        return self._call("getpagesize")

    def getdtablesize(self):
        return self._call("getdtablesize")

    def samefile(self, path1, path2):
        first = self.stat(path1)
        second = self.stat(path2)

        return (
            self._stat_value(first, "st_dev")
            == self._stat_value(second, "st_dev")
            and
            self._stat_value(first, "st_ino")
            == self._stat_value(second, "st_ino")
        )

    def exists(self, path):
        try:
            self.stat(path)
            return True
        except OSError:
            return False

    def lexists(self, path):
        try:
            self.lstat(path)
            return True
        except OSError:
            return False

    def isfile(self, path):
        try:
            return self._is_regular(self.stat(path))
        except OSError:
            return False

    def isdir(self, path):
        try:
            return self._is_directory(self.stat(path))
        except OSError:
            return False

    def islink(self, path):
        try:
            return self._is_link(self.lstat(path))
        except OSError:
            return False

    def getsize(self, path):
        return self._stat_value(
            self.stat(path),
            "st_size",
        )

    def getatime(self, path):
        return self._stat_value(
            self.stat(path),
            "st_atime",
        )

    def getmtime(self, path):
        return self._stat_value(
            self.stat(path),
            "st_mtime",
        )

    def getctime(self, path):
        return self._stat_value(
            self.stat(path),
            "st_ctime",
        )

    def _stat_value(self, info, name):
        if hasattr(info, name):
            return getattr(info, name)

        if isinstance(info, dict) and name in info:
            return info[name]

        raise AttributeError(
            "stat result has no attribute " + name
        )

    def _stat_mode(self, info):
        return self._stat_value(info, "st_mode")

    def _is_regular(self, info):
        method = getattr(info, "is_file", None)

        if method is not None:
            return bool(method())

        if isinstance(info, dict):
            value = info.get("is_file")

            if value is not None:
                return bool(value)

        return (
            self._stat_mode(info) & 0o170000
        ) == 0o100000

    def _is_directory(self, info):
        method = getattr(info, "is_dir", None)

        if method is not None:
            return bool(method())

        if isinstance(info, dict):
            value = info.get("is_dir")

            if value is not None:
                return bool(value)

        return (
            self._stat_mode(info) & 0o170000
        ) == 0o040000

    def _is_link(self, info):
        method = getattr(info, "is_symlink", None)

        if method is not None:
            return bool(method())

        if isinstance(info, dict):
            value = info.get("is_symlink")

            if value is not None:
                return bool(value)

        return (
            self._stat_mode(info) & 0o170000
        ) == 0o120000


def create_backend(native=None):
    return POSIXBackend(native)


__all__ = [
    "POSIXBackend",
    "create_backend",
]
