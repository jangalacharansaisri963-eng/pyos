from __future__ import annotations


_NATIVE = None


class NativePOSIXError(OSError):
    pass


class NativeUnavailableError(RuntimeError):
    pass


class NativeOperationError(OSError):
    pass


class NativePOSIX:
    name = "native-posix"
    platform = "posix"

    def __init__(self, provider=None):
        self._provider = provider

    def set_provider(self, provider):
        self._provider = provider
        return self

    def get_provider(self):
        return self._provider

    def available(self):
        return self._provider is not None

    def require_provider(self):
        provider = self._provider

        if provider is None:
            raise NativeUnavailableError(
                "POSIX native provider has not been initialized"
            )

        return provider

    def _resolve(self, name):
        provider = self.require_provider()

        function = getattr(provider, name, None)

        if function is None or not callable(function):
            raise NotImplementedError(
                "native POSIX provider does not implement "
                + name
            )

        return function

    def call(self, name, *args, **kwargs):
        return self._resolve(name)(*args, **kwargs)

    def has(self, name):
        provider = self._provider

        if provider is None:
            return False

        return callable(getattr(provider, name, None))

    def getpid(self):
        return self.call("getpid")

    def getppid(self):
        return self.call("getppid")

    def getuid(self):
        return self.call("getuid")

    def geteuid(self):
        return self.call("geteuid")

    def getgid(self):
        return self.call("getgid")

    def getegid(self):
        return self.call("getegid")

    def getgroups(self):
        return self.call("getgroups")

    def setuid(self, uid):
        return self.call("setuid", uid)

    def seteuid(self, uid):
        return self.call("seteuid", uid)

    def setgid(self, gid):
        return self.call("setgid", gid)

    def setegid(self, gid):
        return self.call("setegid", gid)

    def getcwd(self):
        return self.call("getcwd")

    def chdir(self, path):
        return self.call("chdir", path)

    def fchdir(self, fd):
        return self.call("fchdir", fd)

    def getwd(self):
        return self.call("getwd")

    def listdir(self, path="."):
        return self.call("listdir", path)

    def scandir(self, path="."):
        return self.call("scandir", path)

    def mkdir(self, path, mode=0o777):
        return self.call("mkdir", path, mode)

    def mkdirat(self, dirfd, path, mode=0o777):
        return self.call("mkdirat", dirfd, path, mode)

    def makedirs(self, path, mode=0o777, exist_ok=False):
        return self.call("makedirs", path, mode, exist_ok)

    def rmdir(self, path):
        return self.call("rmdir", path)

    def removedirs(self, path):
        return self.call("removedirs", path)

    def open(self, path, flags, mode=0o666):
        return self.call("open", path, flags, mode)

    def openat(self, dirfd, path, flags, mode=0o666):
        return self.call("openat", dirfd, path, flags, mode)

    def close(self, fd):
        return self.call("close", fd)

    def read(self, fd, size):
        return self.call("read", fd, size)

    def write(self, fd, data):
        return self.call("write", fd, data)

    def pread(self, fd, size, offset):
        return self.call("pread", fd, size, offset)

    def pwrite(self, fd, data, offset):
        return self.call("pwrite", fd, data, offset)

    def readv(self, fd, buffers):
        return self.call("readv", fd, buffers)

    def writev(self, fd, buffers):
        return self.call("writev", fd, buffers)

    def lseek(self, fd, position, whence=0):
        return self.call("lseek", fd, position, whence)

    def fsync(self, fd):
        return self.call("fsync", fd)

    def fdatasync(self, fd):
        return self.call("fdatasync", fd)

    def ftruncate(self, fd, length):
        return self.call("ftruncate", fd, length)

    def truncate(self, path, length):
        return self.call("truncate", path, length)

    def stat(self, path):
        return self.call("stat", path)

    def lstat(self, path):
        return self.call("lstat", path)

    def fstat(self, fd):
        return self.call("fstat", fd)

    def fstatat(self, dirfd, path, flags=0):
        return self.call("fstatat", dirfd, path, flags)

    def chmod(self, path, mode):
        return self.call("chmod", path, mode)

    def fchmod(self, fd, mode):
        return self.call("fchmod", fd, mode)

    def chown(self, path, uid, gid):
        return self.call("chown", path, uid, gid)

    def fchown(self, fd, uid, gid):
        return self.call("fchown", fd, uid, gid)

    def lchown(self, path, uid, gid):
        return self.call("lchown", path, uid, gid)

    def unlink(self, path):
        return self.call("unlink", path)

    def unlinkat(self, dirfd, path, flags=0):
        return self.call("unlinkat", dirfd, path, flags)

    def rename(self, old, new):
        return self.call("rename", old, new)

    def renameat(self, olddirfd, old, newdirfd, new):
        return self.call(
            "renameat",
            olddirfd,
            old,
            newdirfd,
            new,
        )

    def replace(self, old, new):
        return self.call("replace", old, new)

    def access(self, path, mode):
        return self.call("access", path, mode)

    def faccessat(self, dirfd, path, mode, flags=0):
        return self.call(
            "faccessat",
            dirfd,
            path,
            mode,
            flags,
        )

    def readlink(self, path):
        return self.call("readlink", path)

    def readlinkat(self, dirfd, path):
        return self.call("readlinkat", dirfd, path)

    def symlink(self, src, dst):
        return self.call("symlink", src, dst)

    def symlinkat(self, src, dirfd, dst):
        return self.call("symlinkat", src, dirfd, dst)

    def link(self, src, dst):
        return self.call("link", src, dst)

    def linkat(self, olddirfd, old, newdirfd, new):
        return self.call(
            "linkat",
            olddirfd,
            old,
            newdirfd,
            new,
        )

    def dup(self, fd):
        return self.call("dup", fd)

    def dup2(self, oldfd, newfd):
        return self.call("dup2", oldfd, newfd)

    def dup3(self, oldfd, newfd, flags=0):
        return self.call("dup3", oldfd, newfd, flags)

    def pipe(self):
        return self.call("pipe")

    def pipe2(self, flags=0):
        return self.call("pipe2", flags)

    def fcntl(self, fd, command, argument=0):
        return self.call("fcntl", fd, command, argument)

    def ioctl(self, fd, request, argument=None):
        if argument is None:
            return self.call("ioctl", fd, request)

        return self.call("ioctl", fd, request, argument)

    def isatty(self, fd):
        return self.call("isatty", fd)

    def ttyname(self, fd):
        return self.call("ttyname", fd)

    def sync(self):
        return self.call("sync")

    def syncfs(self, fd):
        return self.call("syncfs", fd)

    def uname(self):
        return self.call("uname")

    def sysconf(self, name):
        return self.call("sysconf", name)

    def pathconf(self, path, name):
        return self.call("pathconf", path, name)

    def fpathconf(self, fd, name):
        return self.call("fpathconf", fd, name)

    def getloadavg(self):
        return self.call("getloadavg")

    def getpagesize(self):
        return self.call("getpagesize")

    def getdtablesize(self):
        return self.call("getdtablesize")

    def sleep(self, seconds):
        return self.call("sleep", seconds)

    def usleep(self, microseconds):
        return self.call("usleep", microseconds)

    def nanosleep(self, seconds, nanoseconds=0):
        return self.call("nanosleep", seconds, nanoseconds)

    def fork(self):
        return self.call("fork")

    def vfork(self):
        return self.call("vfork")

    def waitpid(self, pid, options=0):
        return self.call("waitpid", pid, options)

    def wait(self):
        return self.call("wait")

    def kill(self, pid, signal):
        return self.call("kill", pid, signal)

    def raise_signal(self, signal):
        return self.call("raise_signal", signal)

    def pause(self):
        return self.call("pause")

    def setsid(self):
        return self.call("setsid")

    def getsid(self, pid):
        return self.call("getsid", pid)

    def setpgid(self, pid, pgid):
        return self.call("setpgid", pid, pgid)

    def getpgid(self, pid):
        return self.call("getpgid", pid)

    def getpgrp(self):
        return self.call("getpgrp")

    def execve(self, path, argv, env):
        return self.call("execve", path, argv, env)

    def execv(self, path, argv):
        return self.call("execv", path, argv)

    def execvp(self, file, argv):
        return self.call("execvp", file, argv)

    def execvpe(self, file, argv, env):
        return self.call("execvpe", file, argv, env)

    def _exit(self, status):
        return self.call("_exit", status)

    def poll(self, descriptors, timeout=-1):
        return self.call("poll", descriptors, timeout)

    def select(self, readfds, writefds, exceptfds, timeout=None):
        return self.call(
            "select",
            readfds,
            writefds,
            exceptfds,
            timeout,
        )

    def mmap(self, length, protection, flags, fd=-1, offset=0):
        return self.call(
            "mmap",
            length,
            protection,
            flags,
            fd,
            offset,
        )

    def munmap(self, address, length):
        return self.call("munmap", address, length)

    def mprotect(self, address, length, protection):
        return self.call("mprotect", address, length, protection)

    def msync(self, address, length, flags=0):
        return self.call("msync", address, length, flags)

    def mlock(self, address, length):
        return self.call("mlock", address, length)

    def munlock(self, address, length):
        return self.call("munlock", address, length)

    def socket(self, family, type, protocol=0):
        return self.call("socket", family, type, protocol)

    def socketpair(self, family, type, protocol=0):
        return self.call(
            "socketpair",
            family,
            type,
            protocol,
        )

    def bind(self, fd, address):
        return self.call("bind", fd, address)

    def listen(self, fd, backlog=128):
        return self.call("listen", fd, backlog)

    def accept(self, fd):
        return self.call("accept", fd)

    def connect(self, fd, address):
        return self.call("connect", fd, address)

    def send(self, fd, data, flags=0):
        return self.call("send", fd, data, flags)

    def recv(self, fd, size, flags=0):
        return self.call("recv", fd, size, flags)

    def shutdown(self, fd, how):
        return self.call("shutdown", fd, how)

    def getsockname(self, fd):
        return self.call("getsockname", fd)

    def getpeername(self, fd):
        return self.call("getpeername", fd)

    def setsockopt(self, fd, level, option, value):
        return self.call(
            "setsockopt",
            fd,
            level,
            option,
            value,
        )

    def getsockopt(self, fd, level, option):
        return self.call(
            "getsockopt",
            fd,
            level,
            option,
        )

    def sysinfo(self):
        return self.call("sysinfo")

    def uptime(self):
        return self.call("uptime")

    def mount(self, source, target, filesystem, flags=0, data=None):
        return self.call(
            "mount",
            source,
            target,
            filesystem,
            flags,
            data,
        )

    def umount(self, target, flags=0):
        return self.call("umount", target, flags)

    def constant(self, name, default=None):
        provider = self.require_provider()

        return getattr(provider, name, default)

    def constants(self):
        provider = self.require_provider()

        values = getattr(provider, "constants", None)

        if values is None:
            return {}

        if callable(values):
            values = values()

        return dict(values)


def create_native(provider=None):
    return NativePOSIX(provider)


def set_native(native):
    global _NATIVE
    _NATIVE = native


def get_native():
    native = _NATIVE

    if native is None:
        raise NativeUnavailableError(
            "native POSIX layer has not been initialized"
        )

    return native


def has_native():
    return _NATIVE is not None


def reset_native():
    global _NATIVE
    _NATIVE = None


def initialize(provider=None):
    native = NativePOSIX(provider)
    set_native(native)
    return native


def call(name, *args, **kwargs):
    return get_native().call(name, *args, **kwargs)


__all__ = [
    "NativePOSIX",
    "NativePOSIXError",
    "NativeOperationError",
    "NativeUnavailableError",
    "create_native",
    "set_native",
    "get_native",
    "has_native",
    "reset_native",
    "initialize",
    "call",
]
