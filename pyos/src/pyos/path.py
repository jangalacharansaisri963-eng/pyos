from __future__ import annotations

from . import _path


name = "posix"

sep = "/"
altsep = None
pathsep = ":"
defpath = ":/bin:/usr/bin"
devnull = "/dev/null"

curdir = "."
pardir = ".."
extsep = "."


F_OK = 0
X_OK = 1
W_OK = 2
R_OK = 4

SEEK_SET = 0
SEEK_CUR = 1
SEEK_END = 2

STDIN_FILENO = 0
STDOUT_FILENO = 1
STDERR_FILENO = 2

O_RDONLY = 0
O_WRONLY = 1
O_RDWR = 2
O_CREAT = 0o100
O_EXCL = 0o200
O_TRUNC = 0o1000
O_APPEND = 0o2000
O_NONBLOCK = 0o4000
O_DIRECTORY = 0o200000
O_NOFOLLOW = 0o400000
O_CLOEXEC = 0o2000000

AT_FDCWD = -100
AT_SYMLINK_NOFOLLOW = 0x100
AT_REMOVEDIR = 0x200
AT_SYMLINK_FOLLOW = 0x400
AT_EMPTY_PATH = 0x1000

WNOHANG = 1
WUNTRACED = 2
WCONTINUED = 8

S_IFMT = 0o170000
S_IFREG = 0o100000
S_IFDIR = 0o040000
S_IFLNK = 0o120000
S_IFCHR = 0o020000
S_IFBLK = 0o060000
S_IFIFO = 0o010000
S_IFSOCK = 0o140000


def _native():
    native = globals().get("_native_provider")

    if native is None:
        raise RuntimeError(
            "pyos native provider has not been initialized"
        )

    return native


def _backend():
    backend = globals().get("_filesystem_backend")

    if backend is None:
        raise RuntimeError(
            "pyos filesystem backend has not been initialized"
        )

    return backend


fspath = _path.fspath
_fspath = _path._fspath

path = _path


def getcwd():
    return _backend().getcwd()


def chdir(pathname):
    return _backend().chdir(fspath(pathname))


def fchdir(fd):
    return _native().fchdir(fd)


def listdir(pathname="."):
    return _backend().listdir(fspath(pathname))


def scandir(pathname="."):
    return _backend().scandir(fspath(pathname))


def mkdir(pathname, mode=0o777):
    return _backend().mkdir(
        fspath(pathname),
        mode,
    )


def makedirs(pathname, mode=0o777, exist_ok=False):
    return _backend().makedirs(
        fspath(pathname),
        mode,
        exist_ok,
    )


def rmdir(pathname):
    return _backend().rmdir(fspath(pathname))


def removedirs(pathname):
    return _backend().removedirs(fspath(pathname))


def remove(pathname):
    return _backend().remove(fspath(pathname))


def unlink(pathname):
    return _backend().unlink(fspath(pathname))


def rename(old, new):
    return _backend().rename(
        fspath(old),
        fspath(new),
    )


def replace(old, new):
    return _backend().replace(
        fspath(old),
        fspath(new),
    )


def stat(pathname):
    return _backend().stat(fspath(pathname))


def lstat(pathname):
    return _backend().lstat(fspath(pathname))


def fstat(fd):
    return _backend().fstat(fd)


def chmod(pathname, mode):
    return _backend().chmod(
        fspath(pathname),
        mode,
    )


def fchmod(fd, mode):
    return _backend().fchmod(fd, mode)


def chown(pathname, uid, gid):
    return _backend().chown(
        fspath(pathname),
        uid,
        gid,
    )


def fchown(fd, uid, gid):
    return _backend().fchown(
        fd,
        uid,
        gid,
    )


def lchown(pathname, uid, gid):
    return _backend().lchown(
        fspath(pathname),
        uid,
        gid,
    )


def access(pathname, mode):
    return _backend().access(
        fspath(pathname),
        mode,
    )


def readlink(pathname):
    return _backend().readlink(
        fspath(pathname)
    )


def symlink(src, dst, target_is_directory=False):
    return _backend().symlink(
        fspath(src),
        fspath(dst),
        target_is_directory,
    )


def link(src, dst):
    return _backend().link(
        fspath(src),
        fspath(dst),
    )


def truncate(pathname, length):
    return _backend().truncate(
        fspath(pathname),
        length,
    )


def open(pathname, flags, mode=0o777):
    return _backend().open(
        fspath(pathname),
        flags,
        mode,
    )


def close(fd):
    return _native().close(fd)


def read(fd, size):
    return _native().read(fd, size)


def write(fd, data):
    return _native().write(fd, data)


def pread(fd, size, offset):
    return _native().pread(
        fd,
        size,
        offset,
    )


def pwrite(fd, data, offset):
    return _native().pwrite(
        fd,
        data,
        offset,
    )


def lseek(fd, position, whence=SEEK_SET):
    return _native().lseek(
        fd,
        position,
        whence,
    )


def fsync(fd):
    return _native().fsync(fd)


def fdatasync(fd):
    return _native().fdatasync(fd)


def ftruncate(fd, length):
    return _native().ftruncate(
        fd,
        length,
    )


def dup(fd):
    return _native().dup(fd)


def dup2(oldfd, newfd):
    return _native().dup2(
        oldfd,
        newfd,
    )


def dup3(oldfd, newfd, flags=0):
    return _native().dup3(
        oldfd,
        newfd,
        flags,
    )


def pipe():
    return _native().pipe()


def pipe2(flags=0):
    return _native().pipe2(flags)


def fcntl(fd, command, argument=0):
    return _native().fcntl(
        fd,
        command,
        argument,
    )


def ioctl(fd, request, argument=None):
    if argument is None:
        return _native().ioctl(
            fd,
            request,
        )

    return _native().ioctl(
        fd,
        request,
        argument,
    )


def isatty(fd):
    return _native().isatty(fd)


def ttyname(fd):
    return _native().ttyname(fd)


def sync():
    return _native().sync()


def syncfs(fd):
    return _native().syncfs(fd)


def uname():
    return _native().uname()


def sysconf(name):
    return _native().sysconf(name)


def pathconf(pathname, name):
    return _native().pathconf(
        fspath(pathname),
        name,
    )


def fpathconf(fd, name):
    return _native().fpathconf(fd, name)


def getpid():
    return _native().getpid()


def getppid():
    return _native().getppid()


def getuid():
    return _native().getuid()


def geteuid():
    return _native().geteuid()


def getgid():
    return _native().getgid()


def getegid():
    return _native().getegid()


def getgroups():
    return _native().getgroups()


def setuid(uid):
    return _native().setuid(uid)


def seteuid(uid):
    return _native().seteuid(uid)


def setgid(gid):
    return _native().setgid(gid)


def setegid(gid):
    return _native().setegid(gid)


def fork():
    return _native().fork()


def vfork():
    return _native().vfork()


def waitpid(pid, options=0):
    return _native().waitpid(
        pid,
        options,
    )


def kill(pid, signal):
    return _native().kill(
        pid,
        signal,
    )


def pause():
    return _native().pause()


def setsid():
    return _native().setsid()


def getsid(pid):
    return _native().getsid(pid)


def setpgid(pid, pgid):
    return _native().setpgid(
        pid,
        pgid,
    )


def getpgid(pid):
    return _native().getpgid(pid)


def getpgrp():
    return _native().getpgrp()


def execve(pathname, argv, env):
    return _native().execve(
        fspath(pathname),
        argv,
        env,
    )


def execv(pathname, argv):
    return _native().execv(
        fspath(pathname),
        argv,
    )


def execvp(file, argv):
    return _native().execvp(
        fspath(file),
        argv,
    )


def execvpe(file, argv, env):
    return _native().execvpe(
        fspath(file),
        argv,
        env,
    )


def _exit(status=0):
    return _native()._exit(status)


def sleep(seconds):
    return _native().sleep(seconds)


def usleep(microseconds):
    return _native().usleep(microseconds)


def nanosleep(seconds, nanoseconds=0):
    return _native().nanosleep(
        seconds,
        nanoseconds,
    )


def poll(descriptors, timeout=-1):
    return _native().poll(
        descriptors,
        timeout,
    )


def select(readfds, writefds, exceptfds, timeout=None):
    return _native().select(
        readfds,
        writefds,
        exceptfds,
        timeout,
    )


def socket(family, type, protocol=0):
    return _native().socket(
        family,
        type,
        protocol,
    )


def bind(fd, address):
    return _native().bind(
        fd,
        address,
    )


def listen(fd, backlog=128):
    return _native().listen(
        fd,
        backlog,
    )


def accept(fd):
    return _native().accept(fd)


def connect(fd, address):
    return _native().connect(
        fd,
        address,
    )


def send(fd, data, flags=0):
    return _native().send(
        fd,
        data,
        flags,
    )


def recv(fd, size, flags=0):
    return _native().recv(
        fd,
        size,
        flags,
    )


def shutdown(fd, how):
    return _native().shutdown(
        fd,
        how,
    )


def getsockname(fd):
    return _native().getsockname(fd)


def getpeername(fd):
    return _native().getpeername(fd)


def setsockopt(fd, level, option, value):
    return _native().setsockopt(
        fd,
        level,
        option,
        value,
    )


def getsockopt(fd, level, option):
    return _native().getsockopt(
        fd,
        level,
        option,
    )


def getloadavg():
    return _native().getloadavg()


def getpagesize():
    return _native().getpagesize()


def getdtablesize():
    return _native().getdtablesize()


def sysinfo():
    return _native().sysinfo()


def mount(source, target, filesystem, flags=0, data=None):
    return _native().mount(
        source,
        fspath(target),
        filesystem,
        flags,
        data,
    )


def umount(target, flags=0):
    return _native().umount(
        fspath(target),
        flags,
    )


def exists(pathname):
    return _path.exists(pathname)


def lexists(pathname):
    return _path.lexists(pathname)


def isfile(pathname):
    return _path.isfile(pathname)


def isdir(pathname):
    return _path.isdir(pathname)


def islink(pathname):
    return _path.islink(pathname)


def samefile(path1, path2):
    return _path.samefile(
        path1,
        path2,
    )


def sameopenfile(fp1, fp2):
    return _path.sameopenfile(
        fp1,
        fp2,
    )


def getsize(pathname):
    return _path.getsize(pathname)


def getatime(pathname):
    return _path.getatime(pathname)


def getmtime(pathname):
    return _path.getmtime(pathname)


def getctime(pathname):
    return _path.getctime(pathname)


def expanduser(pathname):
    return _path.expanduser(pathname)


def expandvars(pathname):
    return _path.expandvars(pathname)


def _initialize(native, filesystem_backend):
    globals()["_native_provider"] = native
    globals()["_filesystem_backend"] = filesystem_backend

    cwd = filesystem_backend.getcwd()

    environ = getattr(
        native,
        "environ",
        None,
    )

    environb = getattr(
        native,
        "environb",
        None,
    )

    _path._set_backend(
        cwd=cwd,
        environ=environ,
        environb=environb,
        filesystem=filesystem_backend,
        stat=filesystem_backend.stat,
    )


def _clear():
    globals().pop("_native_provider", None)
    globals().pop("_filesystem_backend", None)
    _path._clear_backend()


__all__ = [
    "name",
    "sep",
    "altsep",
    "pathsep",
    "defpath",
    "devnull",
    "curdir",
    "pardir",
    "extsep",
    "F_OK",
    "X_OK",
    "W_OK",
    "R_OK",
    "SEEK_SET",
    "SEEK_CUR",
    "SEEK_END",
    "STDIN_FILENO",
    "STDOUT_FILENO",
    "STDERR_FILENO",
    "O_RDONLY",
    "O_WRONLY",
    "O_RDWR",
    "O_CREAT",
    "O_EXCL",
    "O_TRUNC",
    "O_APPEND",
    "O_NONBLOCK",
    "O_DIRECTORY",
    "O_NOFOLLOW",
    "O_CLOEXEC",
    "AT_FDCWD",
    "AT_SYMLINK_NOFOLLOW",
    "AT_REMOVEDIR",
    "AT_SYMLINK_FOLLOW",
    "AT_EMPTY_PATH",
    "WNOHANG",
    "WUNTRACED",
    "WCONTINUED",
    "S_IFMT",
    "S_IFREG",
    "S_IFDIR",
    "S_IFLNK",
    "S_IFCHR",
    "S_IFBLK",
    "S_IFIFO",
    "S_IFSOCK",
    "fspath",
    "path",
    "getcwd",
    "chdir",
    "fchdir",
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
    "fchmod",
    "chown",
    "fchown",
    "lchown",
    "access",
    "readlink",
    "symlink",
    "link",
    "truncate",
    "open",
    "close",
    "read",
    "write",
    "pread",
    "pwrite",
    "lseek",
    "fsync",
    "fdatasync",
    "ftruncate",
    "dup",
    "dup2",
    "dup3",
    "pipe",
    "pipe2",
    "fcntl",
    "ioctl",
    "isatty",
    "ttyname",
    "sync",
    "syncfs",
    "uname",
    "sysconf",
    "pathconf",
    "fpathconf",
    "getpid",
    "getppid",
    "getuid",
    "geteuid",
    "getgid",
    "getegid",
    "getgroups",
    "setuid",
    "seteuid",
    "setgid",
    "setegid",
    "fork",
    "vfork",
    "waitpid",
    "kill",
    "pause",
    "setsid",
    "getsid",
    "setpgid",
    "getpgid",
    "getpgrp",
    "execve",
    "execv",
    "execvp",
    "execvpe",
    "_exit",
    "sleep",
    "usleep",
    "nanosleep",
    "poll",
    "select",
    "socket",
    "bind",
    "listen",
    "accept",
    "connect",
    "send",
    "recv",
    "shutdown",
    "getsockname",
    "getpeername",
    "setsockopt",
    "getsockopt",
    "getloadavg",
    "getpagesize",
    "getdtablesize",
    "sysinfo",
    "mount",
    "umount",
    "exists",
    "lexists",
    "isfile",
    "isdir",
    "islink",
    "samefile",
    "sameopenfile",
    "getsize",
    "getatime",
    "getmtime",
    "getctime",
    "expanduser",
    "expandvars",
]
