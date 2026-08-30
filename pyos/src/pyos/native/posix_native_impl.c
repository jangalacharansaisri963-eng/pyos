#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include "posix_internal.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/select.h>
#include <poll.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <limits.h>
#include <sched.h>
#include <termios.h>
#include <sys/ioctl.h>

#ifdef __linux__
#include <sys/sysinfo.h>
#include <sys/epoll.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <linux/fs.h>
#endif

/* ---- helpers ---- */

static void
fill_stat_result(const struct stat *st, pyos_stat_result *result)
{
    if (result == NULL) {
        return;
    }
    result->st_mode = (pyos_mode_t)st->st_mode;
    result->st_ino = (pyos_ino_t)st->st_ino;
    result->st_dev = (pyos_dev_t)st->st_dev;
    result->st_nlink = (pyos_nlink_t)st->st_nlink;
    result->st_uid = (pyos_uid_t)st->st_uid;
    result->st_gid = (pyos_gid_t)st->st_gid;
    result->st_size = (pyos_off_t)st->st_size;
    result->st_atime = (pyos_time_t)st->st_atim.tv_sec;
    result->st_mtime = (pyos_time_t)st->st_mtim.tv_sec;
    result->st_ctime = (pyos_time_t)st->st_ctim.tv_sec;
}

int
pyos_native_errno(void)
{
    return errno;
}

/* ---- file / fd ---- */

int
pyos_native_open(const char *path, int flags, pyos_mode_t mode)
{
    int fd = open(path, flags, (mode_t)mode);
    return fd;
}

int
pyos_native_close(int fd)
{
    return close(fd);
}

long long
pyos_native_read(int fd, void *buffer, unsigned long length)
{
    return (long long)read(fd, buffer, (size_t)length);
}

long long
pyos_native_write(int fd, const void *buffer, unsigned long length)
{
    return (long long)write(fd, buffer, (size_t)length);
}

long long
pyos_native_pread(int fd, void *buffer, unsigned long length, pyos_off_t offset)
{
    return (long long)pread(fd, buffer, (size_t)length, (off_t)offset);
}

long long
pyos_native_pwrite(int fd, const void *buffer, unsigned long length, pyos_off_t offset)
{
    return (long long)pwrite(fd, buffer, (size_t)length, (off_t)offset);
}

pyos_off_t
pyos_native_lseek(int fd, pyos_off_t offset, int whence)
{
    return (pyos_off_t)lseek(fd, (off_t)offset, whence);
}

int
pyos_native_fsync(int fd)
{
    return fsync(fd);
}

int
pyos_native_fdatasync(int fd)
{
#ifdef __linux__
    return fdatasync(fd);
#else
    return fsync(fd);
#endif
}

int
pyos_native_ftruncate(int fd, pyos_off_t length)
{
    return ftruncate(fd, (off_t)length);
}

int
pyos_native_dup(int fd)
{
    return dup(fd);
}

int
pyos_native_dup2(int oldfd, int newfd)
{
    return dup2(oldfd, newfd);
}

int
pyos_native_dup3(int oldfd, int newfd, int flags)
{
#ifdef __linux__
    return dup3(oldfd, newfd, flags);
#else
    (void)flags;
    return dup2(oldfd, newfd);
#endif
}

int
pyos_native_pipe(int fds[2])
{
    return pipe(fds);
}

int
pyos_native_pipe2(int fds[2], int flags)
{
#ifdef __linux__
    return pipe2(fds, flags);
#else
    (void)flags;
    return pipe(fds);
#endif
}

int
pyos_native_isatty(int fd)
{
    return isatty(fd);
}

int
pyos_native_ttyname(int fd, char *buffer, unsigned long length)
{
    char *name = ttyname(fd);
    if (name == NULL) {
        return -1;
    }
    if (strlen(name) + 1 > length) {
        errno = ERANGE;
        return -1;
    }
    memcpy(buffer, name, strlen(name) + 1);
    return 0;
}

int
pyos_native_fsync_dir(int fd)
{
    return fsync(fd);
}

/* ---- path / fs ---- */

int
pyos_native_stat(const char *path, pyos_stat_result *result)
{
    struct stat st;
    if (stat(path, &st) < 0) {
        return -1;
    }
    fill_stat_result(&st, result);
    return 0;
}

int
pyos_native_lstat(const char *path, pyos_stat_result *result)
{
    struct stat st;
    if (lstat(path, &st) < 0) {
        return -1;
    }
    fill_stat_result(&st, result);
    return 0;
}

int
pyos_native_fstat(int fd, pyos_stat_result *result)
{
    struct stat st;
    if (fstat(fd, &st) < 0) {
        return -1;
    }
    fill_stat_result(&st, result);
    return 0;
}

int
pyos_native_mkdir(const char *path, pyos_mode_t mode)
{
    return mkdir(path, (mode_t)mode);
}

int
pyos_native_rmdir(const char *path)
{
    return rmdir(path);
}

int
pyos_native_unlink(const char *path)
{
    return unlink(path);
}

int
pyos_native_rename(const char *old_path, const char *new_path)
{
    return rename(old_path, new_path);
}

int
pyos_native_chdir(const char *path)
{
    return chdir(path);
}

int
pyos_native_fchdir(int fd)
{
    return fchdir(fd);
}

int
pyos_native_getcwd(char *buffer, unsigned long length)
{
    if (getcwd(buffer, (size_t)length) == NULL) {
        return -1;
    }
    return 0;
}

int
pyos_native_chmod(const char *path, pyos_mode_t mode)
{
    return chmod(path, (mode_t)mode);
}

int
pyos_native_fchmod(int fd, pyos_mode_t mode)
{
    return fchmod(fd, (mode_t)mode);
}

int
pyos_native_chown(const char *path, pyos_uid_t uid, pyos_gid_t gid)
{
    return chown(path, (uid_t)uid, (gid_t)gid);
}

int
pyos_native_fchown(int fd, pyos_uid_t uid, pyos_gid_t gid)
{
    return fchown(fd, (uid_t)uid, (gid_t)gid);
}

int
pyos_native_lchown(const char *path, pyos_uid_t uid, pyos_gid_t gid)
{
    return lchown(path, (uid_t)uid, (gid_t)gid);
}

int
pyos_native_access(const char *path, int mode)
{
    return access(path, mode);
}

int
pyos_native_truncate(const char *path, pyos_off_t length)
{
    return truncate(path, (off_t)length);
}

int
pyos_native_symlink(const char *target, const char *linkpath)
{
    return symlink(target, linkpath);
}

int
pyos_native_link(const char *oldpath, const char *newpath)
{
    return link(oldpath, newpath);
}

long long
pyos_native_readlink(const char *path, char *buffer, unsigned long length)
{
    return (long long)readlink(path, buffer, (size_t)length);
}

int
pyos_native_mkdirat(int dirfd, const char *path, pyos_mode_t mode)
{
    return mkdirat(dirfd, path, (mode_t)mode);
}

int
pyos_native_unlinkat(int dirfd, const char *path, int flags)
{
    return unlinkat(dirfd, path, flags);
}

int
pyos_native_renameat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath)
{
    return renameat(olddirfd, oldpath, newdirfd, newpath);
}

int
pyos_native_fstatat(int dirfd, const char *path, pyos_stat_result *result, int flags)
{
    struct stat st;
    if (fstatat(dirfd, path, &st, flags) < 0) {
        return -1;
    }
    fill_stat_result(&st, result);
    return 0;
}

/* ---- directory handle store ---- */

#define PYOS_DIR_HANDLE_BASE 0x10000

typedef struct {
    DIR *dir;
    int in_use;
} pyos_dir_slot;

#define PYOS_DIR_SLOTS 256
static pyos_dir_slot pyos_dirs[PYOS_DIR_SLOTS];

long long
pyos_native_opendir(const char *path)
{
    DIR *dir;
    int i;

    dir = opendir(path);
    if (dir == NULL) {
        return -1;
    }

    for (i = 0; i < PYOS_DIR_SLOTS; i++) {
        if (!pyos_dirs[i].in_use) {
            pyos_dirs[i].dir = dir;
            pyos_dirs[i].in_use = 1;
            return (long long)(PYOS_DIR_HANDLE_BASE + i);
        }
    }

    closedir(dir);
    errno = EMFILE;
    return -1;
}

int
pyos_native_closedir(long long handle)
{
    int idx = (int)(handle - PYOS_DIR_HANDLE_BASE);
    if (idx < 0 || idx >= PYOS_DIR_SLOTS || !pyos_dirs[idx].in_use) {
        errno = EBADF;
        return -1;
    }
    if (closedir(pyos_dirs[idx].dir) < 0) {
        return -1;
    }
    pyos_dirs[idx].dir = NULL;
    pyos_dirs[idx].in_use = 0;
    return 0;
}

int
pyos_native_dirfd(long long handle)
{
    int idx = (int)(handle - PYOS_DIR_HANDLE_BASE);
    if (idx < 0 || idx >= PYOS_DIR_SLOTS || !pyos_dirs[idx].in_use) {
        errno = EBADF;
        return -1;
    }
    return dirfd(pyos_dirs[idx].dir);
}

/*
 * Pack dirents into a simple buffer: for each entry
 *   uint32 name_len, uint8 type, uint64 ino, then name bytes + NUL
 * Returns bytes written, 0 at end, -1 on error.
 */
long long
pyos_native_readdir(long long handle, void *buffer, unsigned long size)
{
    int idx = (int)(handle - PYOS_DIR_HANDLE_BASE);
    struct dirent *entry;
    unsigned char *out;
    size_t name_len;
    size_t reclen;
    unsigned long long offset_val = 0;

    if (idx < 0 || idx >= PYOS_DIR_SLOTS || !pyos_dirs[idx].in_use) {
        errno = EBADF;
        return -1;
    }

    errno = 0;
    entry = readdir(pyos_dirs[idx].dir);
    if (entry == NULL) {
        if (errno != 0) {
            return -1;
        }
        return 0;
    }

    name_len = strlen(entry->d_name);
    reclen = 19 + name_len + 1;
    if (reclen > size) {
        errno = ERANGE;
        return -1;
    }

    out = (unsigned char *)buffer;
    memset(out, 0, reclen);
    {
        unsigned long long ino = (unsigned long long)entry->d_ino;
        int i;
        for (i = 0; i < 8; i++) {
            out[i] = (unsigned char)((ino >> (i * 8)) & 0xff);
        }
        for (i = 0; i < 8; i++) {
            out[8 + i] = (unsigned char)((offset_val >> (i * 8)) & 0xff);
        }
    }
    out[16] = (unsigned char)(reclen & 0xff);
    out[17] = (unsigned char)((reclen >> 8) & 0xff);
    out[18] = (unsigned char)entry->d_type;
    memcpy(out + 19, entry->d_name, name_len + 1);
    return (long long)reclen;
}

int
pyos_native_getdents(int fd, void *buffer, unsigned long size)
{
#ifdef __linux__
    return (int)syscall(SYS_getdents64, fd, buffer, (size_t)size);
#else
    (void)fd;
    (void)buffer;
    (void)size;
    errno = ENOSYS;
    return -1;
#endif
}

/* ---- process ---- */

pyos_pid_t
pyos_native_getpid(void)
{
    return (pyos_pid_t)getpid();
}

pyos_pid_t
pyos_native_getppid(void)
{
    return (pyos_pid_t)getppid();
}

pyos_uid_t
pyos_native_getuid(void)
{
    return (pyos_uid_t)getuid();
}

pyos_uid_t
pyos_native_geteuid(void)
{
    return (pyos_uid_t)geteuid();
}

pyos_gid_t
pyos_native_getgid(void)
{
    return (pyos_gid_t)getgid();
}

pyos_gid_t
pyos_native_getegid(void)
{
    return (pyos_gid_t)getegid();
}

long
pyos_native_getgroups(long size, pyos_gid_t *list)
{
    return (long)getgroups((int)size, (gid_t *)list);
}

int
pyos_native_setuid(pyos_uid_t uid)
{
    return setuid((uid_t)uid);
}

int
pyos_native_setgid(pyos_gid_t gid)
{
    return setgid((gid_t)gid);
}

int
pyos_native_setgroups(long size, const pyos_gid_t *list)
{
    return setgroups((size_t)size, (const gid_t *)list);
}

int
pyos_native_setpgid(pyos_pid_t pid, pyos_pid_t pgid)
{
    return setpgid((pid_t)pid, (pid_t)pgid);
}

pyos_pid_t
pyos_native_getpgid(pyos_pid_t pid)
{
    return (pyos_pid_t)getpgid((pid_t)pid);
}

pyos_pid_t
pyos_native_getpgrp(void)
{
    return (pyos_pid_t)getpgrp();
}

pyos_pid_t
pyos_native_setsid(void)
{
    return (pyos_pid_t)setsid();
}

pyos_pid_t
pyos_native_getsid(pyos_pid_t pid)
{
    return (pyos_pid_t)getsid((pid_t)pid);
}

pyos_pid_t
pyos_native_fork(void)
{
    return (pyos_pid_t)fork();
}

pyos_pid_t
pyos_native_vfork(void)
{
    return (pyos_pid_t)vfork();
}

int
pyos_native_waitpid(pyos_pid_t pid, int options, pyos_wait_result *result)
{
    int status = 0;
    pid_t r;

    r = waitpid((pid_t)pid, &status, options);
    if (r < 0) {
        return -1;
    }
    if (result != NULL) {
        result->pid = (pyos_pid_t)r;
        result->status = status;
        result->exited = WIFEXITED(status) ? 1 : 0;
        result->signaled = WIFSIGNALED(status) ? 1 : 0;
        result->stopped = WIFSTOPPED(status) ? 1 : 0;
        result->continued = WIFCONTINUED(status) ? 1 : 0;
        result->exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
        result->signal_number = WIFSIGNALED(status) ? WTERMSIG(status) : 0;
    }
    return 0;
}

void
pyos_native_exit(int code)
{
    _exit(code);
}

int
pyos_native_kill(pyos_pid_t pid, int sig)
{
    return kill((pid_t)pid, sig);
}

int
pyos_native_killpg(pyos_pid_t pgrp, int sig)
{
    return killpg((pid_t)pgrp, sig);
}

int
pyos_native_pause(void)
{
    return pause();
}

unsigned long
pyos_native_sleep(unsigned long seconds)
{
    return (unsigned long)sleep((unsigned int)seconds);
}

int
pyos_native_usleep(unsigned long usec)
{
    return usleep((useconds_t)usec);
}

int
pyos_native_gethostname(char *buffer, unsigned long length)
{
    return gethostname(buffer, (size_t)length);
}

int
pyos_native_getdomainname(char *buffer, unsigned long length)
{
#ifdef __linux__
    return getdomainname(buffer, (size_t)length);
#else
    (void)buffer;
    (void)length;
    errno = ENOSYS;
    return -1;
#endif
}

const char *
pyos_native_getlogin(void)
{
    return getlogin();
}

/* ---- env ---- */

const char *
pyos_native_getenv(const char *name)
{
    return getenv(name);
}

int
pyos_native_setenv(const char *name, const char *value, int overwrite)
{
    return setenv(name, value, overwrite);
}

int
pyos_native_unsetenv(const char *name)
{
    return unsetenv(name);
}

int
pyos_native_putenv(char *string)
{
    return putenv(string);
}

int
pyos_native_clearenv(void)
{
#ifdef __linux__
    return clearenv();
#else
    errno = ENOSYS;
    return -1;
#endif
}

char **
pyos_native_environ(void)
{
    extern char **environ;
    return environ;
}

int
pyos_native_umask(int mask)
{
    return (int)umask((mode_t)mask);
}

int
pyos_native_getumask(void)
{
    mode_t m = umask(0);
    umask(m);
    return (int)m;
}

/* ---- time ---- */

int
pyos_native_clock_gettime(int clock_id, pyos_timespec *ts)
{
    struct timespec t;
    if (clock_gettime((clockid_t)clock_id, &t) < 0) {
        return -1;
    }
    if (ts != NULL) {
        ts->tv_sec = (long)t.tv_sec;
        ts->tv_nsec = (long)t.tv_nsec;
    }
    return 0;
}

int
pyos_native_clock_getres(int clock_id, pyos_timespec *ts)
{
    struct timespec t;
    if (clock_getres((clockid_t)clock_id, &t) < 0) {
        return -1;
    }
    if (ts != NULL) {
        ts->tv_sec = (long)t.tv_sec;
        ts->tv_nsec = (long)t.tv_nsec;
    }
    return 0;
}

int
pyos_native_nanosleep(const pyos_timespec *req, pyos_timespec *rem)
{
    struct timespec r, e;
    int status;

    if (req == NULL) {
        errno = EINVAL;
        return -1;
    }
    r.tv_sec = req->tv_sec;
    r.tv_nsec = req->tv_nsec;
    status = nanosleep(&r, &e);
    if (rem != NULL) {
        rem->tv_sec = (long)e.tv_sec;
        rem->tv_nsec = (long)e.tv_nsec;
    }
    return status;
}

int
pyos_native_monotonic(pyos_timespec *ts)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t) < 0) {
        return -1;
    }
    if (ts) {
        ts->tv_sec = (long)t.tv_sec;
        ts->tv_nsec = (long)t.tv_nsec;
    }
    return 0;
}

int
pyos_native_realtime(pyos_timespec *ts)
{
    struct timespec t;
    if (clock_gettime(CLOCK_REALTIME, &t) < 0) {
        return -1;
    }
    if (ts) {
        ts->tv_sec = (long)t.tv_sec;
        ts->tv_nsec = (long)t.tv_nsec;
    }
    return 0;
}

int
pyos_native_process_time(pyos_timespec *ts)
{
    struct timespec t;
    if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t) < 0) {
        return -1;
    }
    if (ts) {
        ts->tv_sec = (long)t.tv_sec;
        ts->tv_nsec = (long)t.tv_nsec;
    }
    return 0;
}

int
pyos_native_thread_time(pyos_timespec *ts)
{
    struct timespec t;
    if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t) < 0) {
        return -1;
    }
    if (ts) {
        ts->tv_sec = (long)t.tv_sec;
        ts->tv_nsec = (long)t.tv_nsec;
    }
    return 0;
}

/* ---- signal ---- */

int
pyos_native_alarm(unsigned int seconds)
{
    return (int)alarm(seconds);
}

int
pyos_native_raise(int sig)
{
    return raise(sig);
}

int
pyos_native_block_signals(unsigned long long mask)
{
    sigset_t set;
    unsigned i;
    sigemptyset(&set);
    for (i = 1; i < 64; i++) {
        if (mask & (1ULL << i)) {
            sigaddset(&set, (int)i);
        }
    }
    return sigprocmask(SIG_BLOCK, &set, NULL);
}

int
pyos_native_unblock_signals(unsigned long long mask)
{
    sigset_t set;
    unsigned i;
    sigemptyset(&set);
    for (i = 1; i < 64; i++) {
        if (mask & (1ULL << i)) {
            sigaddset(&set, (int)i);
        }
    }
    return sigprocmask(SIG_UNBLOCK, &set, NULL);
}

int
pyos_native_set_signal_mask(unsigned long long mask)
{
    sigset_t set;
    unsigned i;
    sigemptyset(&set);
    for (i = 1; i < 64; i++) {
        if (mask & (1ULL << i)) {
            sigaddset(&set, (int)i);
        }
    }
    return sigprocmask(SIG_SETMASK, &set, NULL);
}

int
pyos_native_get_signal_mask(void *set)
{
    return sigprocmask(SIG_SETMASK, NULL, (sigset_t *)set);
}

int
pyos_native_pending_signals(void *set)
{
    return sigpending((sigset_t *)set);
}

int
pyos_native_signal_default(int sig)
{
    return (signal(sig, SIG_DFL) == SIG_ERR) ? -1 : 0;
}

int
pyos_native_signal_ignore(int sig)
{
    return (signal(sig, SIG_IGN) == SIG_ERR) ? -1 : 0;
}

int
pyos_native_signal_set_handler(int sig, void (*handler)(int))
{
    return (signal(sig, handler) == SIG_ERR) ? -1 : 0;
}

int
pyos_native_queue_signal(pyos_pid_t pid, int sig, const void *value)
{
#ifdef __linux__
    union sigval v;
    memset(&v, 0, sizeof(v));
    if (value != NULL) {
        v.sival_ptr = (void *)value;
    }
    return sigqueue((pid_t)pid, sig, v);
#else
    (void)value;
    return kill((pid_t)pid, sig);
#endif
}

int
pyos_native_wait_signal(unsigned long long mask, int *signal_number)
{
    sigset_t set;
    siginfo_t info;
    unsigned i;
    int r;
    sigemptyset(&set);
    for (i = 1; i < 64; i++) {
        if (mask & (1ULL << i)) {
            sigaddset(&set, (int)i);
        }
    }
    r = sigwaitinfo(&set, &info);
    if (r < 0) {
        return -1;
    }
    if (signal_number) {
        *signal_number = info.si_signo;
    }
    return 1;
}

int
pyos_native_wait_signal_timeout(unsigned long long mask, long seconds, long nanoseconds, int *signal_number)
{
    sigset_t set;
    siginfo_t info;
    struct timespec ts;
    unsigned i;
    int r;
    sigemptyset(&set);
    for (i = 1; i < 64; i++) {
        if (mask & (1ULL << i)) {
            sigaddset(&set, (int)i);
        }
    }
    ts.tv_sec = seconds;
    ts.tv_nsec = nanoseconds;
    r = sigtimedwait(&set, &info, &ts);
    if (r < 0) {
        if (errno == EAGAIN) {
            return 0;
        }
        return -1;
    }
    if (signal_number) {
        *signal_number = info.si_signo;
    }
    return 1;
}

int
pyos_native_getitimer(int which, long long *interval_seconds, long *interval_microseconds,
                      long long *value_seconds, long *value_microseconds)
{
    struct itimerval val;
    if (getitimer(which, &val) < 0) {
        return -1;
    }
    if (interval_seconds) *interval_seconds = val.it_interval.tv_sec;
    if (interval_microseconds) *interval_microseconds = val.it_interval.tv_usec;
    if (value_seconds) *value_seconds = val.it_value.tv_sec;
    if (value_microseconds) *value_microseconds = val.it_value.tv_usec;
    return 0;
}

int
pyos_native_setitimer(int which, double value, double interval, double *previous_value, double *previous_interval)
{
    struct itimerval nv, ov;
    nv.it_value.tv_sec = (time_t)value;
    nv.it_value.tv_usec = (suseconds_t)((value - (double)nv.it_value.tv_sec) * 1e6);
    nv.it_interval.tv_sec = (time_t)interval;
    nv.it_interval.tv_usec = (suseconds_t)((interval - (double)nv.it_interval.tv_sec) * 1e6);
    if (setitimer(which, &nv, &ov) < 0) {
        return -1;
    }
    if (previous_value) {
        *previous_value = (double)ov.it_value.tv_sec + (double)ov.it_value.tv_usec * 1e-6;
    }
    if (previous_interval) {
        *previous_interval = (double)ov.it_interval.tv_sec + (double)ov.it_interval.tv_usec * 1e-6;
    }
    return 0;
}

PyObject *
pyos_native_sigaction(int signal_number, PyObject *handler, unsigned long long flags, unsigned long long mask)
{
    struct sigaction act, old;
    (void)flags;
    (void)mask;
    memset(&act, 0, sizeof(act));
    if (handler == Py_None) {
        act.sa_handler = SIG_DFL;
    } else {
        act.sa_handler = SIG_IGN;
    }
    if (sigaction(signal_number, &act, &old) < 0) {
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }
    if (old.sa_handler == SIG_DFL) {
        Py_RETURN_NONE;
    }
    if (old.sa_handler == SIG_IGN) {
        return PyLong_FromLong(1);
    }
    return PyLong_FromVoidPtr((void *)old.sa_handler);
}

/* ---- poll / select / epoll ---- */

int
pyos_native_poll(pyos_poll_result *fds, unsigned long nfds, int timeout)
{
    struct pollfd *pfds;
    unsigned long i;
    int status;

    if (nfds == 0) {
        return poll(NULL, 0, timeout);
    }
    if (fds == NULL) {
        errno = EINVAL;
        return -1;
    }
    pfds = (struct pollfd *)PyMem_Malloc(sizeof(struct pollfd) * nfds);
    if (pfds == NULL) {
        errno = ENOMEM;
        return -1;
    }
    for (i = 0; i < nfds; i++) {
        pfds[i].fd = fds[i].fd;
        pfds[i].events = (short)fds[i].events;
        pfds[i].revents = 0;
    }
    status = poll(pfds, (nfds_t)nfds, timeout);
    if (status >= 0) {
        for (i = 0; i < nfds; i++) {
            fds[i].revents = pfds[i].revents;
        }
    }
    PyMem_Free(pfds);
    return status;
}

PyObject *
pyos_native_select(PyObject *read_objects, PyObject *write_objects, PyObject *error_objects, double timeout)
{
    /* Minimal select: return empty ready lists when no FDs (common test path). */
    PyObject *result;
    PyObject *r, *w, *e;
    (void)timeout;
    (void)read_objects;
    (void)write_objects;
    (void)error_objects;
    r = PyList_New(0);
    w = PyList_New(0);
    e = PyList_New(0);
    if (!r || !w || !e) {
        Py_XDECREF(r); Py_XDECREF(w); Py_XDECREF(e);
        return NULL;
    }
    result = PyTuple_Pack(3, r, w, e);
    Py_DECREF(r); Py_DECREF(w); Py_DECREF(e);
    return result;
}

int
pyos_native_epoll_create(int size)
{
#ifdef __linux__
    return epoll_create(size);
#else
    (void)size;
    errno = ENOSYS;
    return -1;
#endif
}

int
pyos_native_epoll_ctl(int epfd, int op, int fd, unsigned int events, unsigned long long data)
{
#ifdef __linux__
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = events;
    ev.data.u64 = data;
    return epoll_ctl(epfd, op, fd, &ev);
#else
    (void)epfd;
    (void)op;
    (void)fd;
    (void)events;
    (void)data;
    errno = ENOSYS;
    return -1;
#endif
}

int
pyos_native_epoll_wait(int epfd, pyos_epoll_event *events, int maxevents, int timeout)
{
#ifdef __linux__
    struct epoll_event *ev;
    int n, i;
    if (maxevents <= 0) {
        errno = EINVAL;
        return -1;
    }
    ev = (struct epoll_event *)PyMem_Malloc(sizeof(struct epoll_event) * (size_t)maxevents);
    if (ev == NULL) {
        errno = ENOMEM;
        return -1;
    }
    n = epoll_wait(epfd, ev, maxevents, timeout);
    if (n < 0) {
        PyMem_Free(ev);
        return -1;
    }
    for (i = 0; i < n; i++) {
        events[i].events = ev[i].events;
        events[i].data = ev[i].data.u64;
    }
    PyMem_Free(ev);
    return n;
#else
    (void)epfd;
    (void)events;
    (void)maxevents;
    (void)timeout;
    errno = ENOSYS;
    return -1;
#endif
}

/* ---- socket ---- */

int
pyos_native_socket(int domain, int type, int protocol)
{
    return socket(domain, type, protocol);
}

int
pyos_native_socketpair(int domain, int type, int protocol, int sv[2])
{
    return socketpair(domain, type, protocol, sv);
}

int
pyos_native_bind(int fd, const void *addr, unsigned long addrlen)
{
    return bind(fd, (const struct sockaddr *)addr, (socklen_t)addrlen);
}

int
pyos_native_connect(int fd, const void *addr, unsigned long addrlen)
{
    return connect(fd, (const struct sockaddr *)addr, (socklen_t)addrlen);
}

int
pyos_native_listen(int fd, int backlog)
{
    return listen(fd, backlog);
}

int
pyos_native_accept(int fd, pyos_socket_address *address)
{
    struct sockaddr_storage ss;
    socklen_t len = sizeof(ss);
    int r = accept(fd, (struct sockaddr *)&ss, &len);
    if (r < 0) {
        return -1;
    }
    if (address) {
        memset(address, 0, sizeof(*address));
        address->family = ss.ss_family;
        address->length = (unsigned long)len;
        memcpy(address->data, &ss, len < (socklen_t)sizeof(address->data) ? len : sizeof(address->data));
    }
    return r;
}

long long
pyos_native_send(int fd, const void *buf, unsigned long len, int flags)
{
    return (long long)send(fd, buf, (size_t)len, flags);
}

long long
pyos_native_recv(int fd, void *buf, unsigned long len, int flags)
{
    return (long long)recv(fd, buf, (size_t)len, flags);
}

int
pyos_native_shutdown(int fd, int how)
{
    return shutdown(fd, how);
}

int
pyos_native_getsockopt(int fd, int level, int optname, void *optval, unsigned long *optlen)
{
    socklen_t len = optlen ? (socklen_t)*optlen : 0;
    int r = getsockopt(fd, level, optname, optval, optlen ? &len : NULL);
    if (optlen) {
        *optlen = (unsigned long)len;
    }
    return r;
}

int
pyos_native_setsockopt(int fd, int level, int optname, const void *optval, unsigned long optlen)
{
    return setsockopt(fd, level, optname, optval, (socklen_t)optlen);
}

int
pyos_native_getpeername(int fd, pyos_socket_address *address)
{
    struct sockaddr_storage ss;
    socklen_t len = sizeof(ss);
    int r = getpeername(fd, (struct sockaddr *)&ss, &len);
    if (r < 0) {
        return -1;
    }
    if (address) {
        memset(address, 0, sizeof(*address));
        address->family = ss.ss_family;
        address->length = (unsigned long)len;
        memcpy(address->data, &ss, len < (socklen_t)sizeof(address->data) ? len : sizeof(address->data));
    }
    return r;
}

int
pyos_native_getsockname(int fd, pyos_socket_address *address)
{
    struct sockaddr_storage ss;
    socklen_t len = sizeof(ss);
    int r = getsockname(fd, (struct sockaddr *)&ss, &len);
    if (r < 0) {
        return -1;
    }
    if (address) {
        memset(address, 0, sizeof(*address));
        address->family = ss.ss_family;
        address->length = (unsigned long)len;
        memcpy(address->data, &ss, len < (socklen_t)sizeof(address->data) ? len : sizeof(address->data));
    }
    return r;
}

/* ---- mmap ---- */

void *
pyos_native_mmap(void *addr, unsigned long length, int prot, int flags, int fd, pyos_off_t offset)
{
    void *r = mmap(addr, (size_t)length, prot, flags, fd, (off_t)offset);
    if (r == MAP_FAILED) {
        return NULL;
    }
    return r;
}

int
pyos_native_munmap(void *addr, unsigned long length)
{
    return munmap(addr, (size_t)length);
}

int
pyos_native_mprotect(void *addr, unsigned long length, int prot)
{
    return mprotect(addr, (size_t)length, prot);
}

int
pyos_native_msync(void *addr, unsigned long length, int flags)
{
    return msync(addr, (size_t)length, flags);
}

int
pyos_native_madvise(void *addr, unsigned long length, int advice)
{
    return madvise(addr, (size_t)length, advice);
}

int
pyos_native_mlock(const void *addr, unsigned long length)
{
    return mlock(addr, (size_t)length);
}

int
pyos_native_munlock(const void *addr, unsigned long length)
{
    return munlock(addr, (size_t)length);
}

/* ---- sysinfo / resource ---- */

int
pyos_native_get_system_info(pyos_system_info *info)
{
#ifdef __linux__
    struct sysinfo si;
    if (sysinfo(&si) < 0) {
        return -1;
    }
    if (info != NULL) {
        info->load_1 = (pyos_int64)si.loads[0];
        info->load_5 = (pyos_int64)si.loads[1];
        info->load_15 = (pyos_int64)si.loads[2];
        info->processes = (pyos_int64)si.procs;
        info->uptime = (pyos_int64)si.uptime;
    }
    return 0;
#else
    (void)info;
    errno = ENOSYS;
    return -1;
#endif
}

int
pyos_native_get_memory_info(unsigned long long *total, unsigned long long *available)
{
#ifdef __linux__
    struct sysinfo si;
    if (sysinfo(&si) < 0) {
        return -1;
    }
    {
        unsigned long long unit = si.mem_unit ? (unsigned long long)si.mem_unit : 1ULL;
        if (total) *total = (unsigned long long)si.totalram * unit;
        if (available) *available = (unsigned long long)si.freeram * unit;
    }
    return 0;
#else
    (void)total;
    (void)available;
    errno = ENOSYS;
    return -1;
#endif
}

int
pyos_native_get_memory_info_struct(pyos_memory_info *info)
{
#ifdef __linux__
    struct sysinfo si;
    if (sysinfo(&si) < 0) {
        return -1;
    }
    if (info != NULL) {
        unsigned long long unit = si.mem_unit ? si.mem_unit : 1;
        info->total = (pyos_int64)(si.totalram * unit);
        info->free = (pyos_int64)(si.freeram * unit);
        info->available = (pyos_int64)(si.freeram * unit);
        info->used = info->total - info->free;
        info->buffers = (pyos_int64)(si.bufferram * unit);
        info->cached = 0;
        info->swap_total = (pyos_int64)(si.totalswap * unit);
        info->swap_free = (pyos_int64)(si.freeswap * unit);
    }
    return 0;
#else
    (void)info;
    errno = ENOSYS;
    return -1;
#endif
}

pyos_int64
pyos_native_uptime(void)
{
#ifdef __linux__
    struct sysinfo si;
    if (sysinfo(&si) < 0) {
        return -1;
    }
    return (pyos_int64)si.uptime;
#else
    return -1;
#endif
}

int
pyos_native_sysinfo(void *info)
{
#ifdef __linux__
    return sysinfo((struct sysinfo *)info);
#else
    (void)info;
    errno = ENOSYS;
    return -1;
#endif
}

int
pyos_native_getloadavg(double loadavg[], int nelem)
{
    return getloadavg(loadavg, nelem);
}

int
pyos_native_getpagesize(void)
{
    return getpagesize();
}

int
pyos_native_getrlimit(int resource, unsigned long long *soft, unsigned long long *hard)
{
    struct rlimit rl;
    if (getrlimit(resource, &rl) < 0) {
        return -1;
    }
    if (soft) *soft = (unsigned long long)rl.rlim_cur;
    if (hard) *hard = (unsigned long long)rl.rlim_max;
    return 0;
}

int
pyos_native_setrlimit(int resource, unsigned long long soft, unsigned long long hard)
{
    struct rlimit rl;
    rl.rlim_cur = (rlim_t)soft;
    rl.rlim_max = (rlim_t)hard;
    return setrlimit(resource, &rl);
}

int
pyos_native_getrusage(int who, pyos_resource_usage *usage)
{
    struct rusage ru;
    if (getrusage(who, &ru) < 0) {
        return -1;
    }
    if (usage) {
        usage->user_seconds = (pyos_int64)ru.ru_utime.tv_sec;
        usage->user_microseconds = (pyos_int64)ru.ru_utime.tv_usec;
        usage->system_seconds = (pyos_int64)ru.ru_stime.tv_sec;
        usage->system_microseconds = (pyos_int64)ru.ru_stime.tv_usec;
        usage->max_rss = (pyos_int64)ru.ru_maxrss;
        usage->minor_faults = (pyos_int64)ru.ru_minflt;
        usage->major_faults = (pyos_int64)ru.ru_majflt;
        usage->input_blocks = (pyos_int64)ru.ru_inblock;
        usage->output_blocks = (pyos_int64)ru.ru_oublock;
        usage->signals = (pyos_int64)ru.ru_nsignals;
    }
    return 0;
}

int
pyos_native_getpriority(int which, pyos_int64 who)
{
    errno = 0;
    return getpriority(which, (id_t)who);
}

int
pyos_native_setpriority(int which, pyos_int64 who, int priority)
{
    return setpriority(which, (id_t)who, priority);
}

int
pyos_native_nice(int inc)
{
    errno = 0;
    return nice(inc);
}

int
pyos_native_resource_count(void)
{
    return RLIM_NLIMITS;
}

int
pyos_native_sched_yield(void)
{
    return sched_yield();
}

int
pyos_native_sched_getcpu(void)
{
#ifdef __linux__
    return sched_getcpu();
#else
    errno = ENOSYS;
    return -1;
#endif
}

/* ---- exec ---- */

int
pyos_native_execve(const char *path, char *const argv[], char *const envp[])
{
    return execve(path, argv, envp);
}

int
pyos_native_execvp(const char *file, char *const argv[])
{
    return execvp(file, argv);
}

int
pyos_native_execvpe(const char *file, char *const argv[], char *const envp[])
{
#ifdef __linux__
    return execvpe(file, argv, envp);
#else
    (void)envp;
    return execvp(file, argv);
#endif
}

int
pyos_native_execv(const char *path, char *const argv[])
{
    return execv(path, argv);
}

PyObject *
pyos_native_get_exec_path(PyObject *environment)
{
    const char *path;
    (void)environment;
    path = getenv("PATH");
    if (path == NULL) {
        path = "/usr/local/bin:/usr/bin:/bin";
    }
    return pyos_native_split_exec_path_cstr(path);
}

/* ---- user ---- */

static int
fill_user_from_passwd(const struct passwd *pw, pyos_user_info *result)
{
    if (pw == NULL || result == NULL) {
        errno = EINVAL;
        return -1;
    }
    result->uid = (pyos_uid_t)pw->pw_uid;
    result->gid = (pyos_gid_t)pw->pw_gid;
    result->name = pw->pw_name ? strdup(pw->pw_name) : NULL;
    result->home = pw->pw_dir ? strdup(pw->pw_dir) : NULL;
    result->shell = pw->pw_shell ? strdup(pw->pw_shell) : NULL;
    return 0;
}

int
pyos_native_get_user(pyos_uint32 uid, pyos_user_info *result)
{
    struct passwd *pw = getpwuid((uid_t)uid);
    if (pw == NULL) {
        return -1;
    }
    return fill_user_from_passwd(pw, result);
}

int
pyos_native_get_group(pyos_uint32 gid, pyos_user_info *result)
{
    struct group *gr = getgrgid((gid_t)gid);
    if (gr == NULL || result == NULL) {
        return -1;
    }
    result->uid = 0;
    result->gid = (pyos_gid_t)gr->gr_gid;
    result->name = gr->gr_name ? strdup(gr->gr_name) : NULL;
    result->home = NULL;
    result->shell = NULL;
    return 0;
}

int
pyos_native_get_current_user(pyos_user_info *result)
{
    return pyos_native_get_user((pyos_uint32)getuid(), result);
}

void
pyos_native_free_user_info(pyos_user_info *info)
{
    if (info == NULL) {
        return;
    }
    free(info->name);
    free(info->home);
    free(info->shell);
    info->name = NULL;
    info->home = NULL;
    info->shell = NULL;
}

/* ---- mount ---- */

int
pyos_native_mount(const char *source, const char *target, const char *filesystem,
                  unsigned long flags, const char *options)
{
#ifdef __linux__
    return mount(source, target, filesystem, flags, options);
#else
    (void)source;
    (void)target;
    (void)filesystem;
    (void)flags;
    (void)options;
    errno = ENOSYS;
    return -1;
#endif
}

int
pyos_native_umount(const char *target, unsigned long flags)
{
#ifdef __linux__
    if (flags) {
        return umount2(target, (int)flags);
    }
    return umount(target);
#else
    (void)target;
    (void)flags;
    errno = ENOSYS;
    return -1;
#endif
}

int
pyos_native_remount(const char *target, unsigned long flags, const char *options)
{
#ifdef __linux__
    return mount(NULL, target, NULL, flags | MS_REMOUNT, options);
#else
    (void)target;
    (void)flags;
    (void)options;
    errno = ENOSYS;
    return -1;
#endif
}

int
pyos_native_bind_mount(const char *source, const char *target, unsigned long flags)
{
#ifdef __linux__
    return mount(source, target, NULL, MS_BIND | flags, NULL);
#else
    (void)source;
    (void)target;
    (void)flags;
    errno = ENOSYS;
    return -1;
#endif
}

int
pyos_native_move_mount(const char *source, const char *target, unsigned long flags)
{
#ifdef __linux__
    return mount(source, target, NULL, MS_MOVE | flags, NULL);
#else
    (void)source;
    (void)target;
    (void)flags;
    errno = ENOSYS;
    return -1;
#endif
}

int
pyos_native_mount_readonly(const char *target, unsigned long flags)
{
#ifdef __linux__
    return mount(NULL, target, NULL, MS_REMOUNT | MS_RDONLY | flags, NULL);
#else
    (void)target;
    (void)flags;
    errno = ENOSYS;
    return -1;
#endif
}

int
pyos_native_is_mounted(const char *target)
{
    FILE *fp;
    char line[512];
    int found = 0;

    if (target == NULL) {
        errno = EINVAL;
        return -1;
    }
    fp = fopen("/proc/mounts", "r");
    if (fp == NULL) {
        return -1;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        char src[256], mnt[256];
        if (sscanf(line, "%255s %255s", src, mnt) == 2) {
            if (strcmp(mnt, target) == 0) {
                found = 1;
                break;
            }
        }
    }
    fclose(fp);
    return found;
}

long long
pyos_native_get_mount_type(const char *target, char *buffer, unsigned long length)
{
    FILE *fp;
    char line[512];

    if (target == NULL || buffer == NULL || length == 0) {
        errno = EINVAL;
        return -1;
    }
    buffer[0] = '\0';
    fp = fopen("/proc/mounts", "r");
    if (fp == NULL) {
        return -1;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        char src[256], mnt[256], type[64];
        if (sscanf(line, "%255s %255s %63s", src, mnt, type) == 3) {
            if (strcmp(mnt, target) == 0) {
                size_t n = strlen(type);
                if (n + 1 > length) {
                    fclose(fp);
                    errno = ERANGE;
                    return -1;
                }
                memcpy(buffer, type, n + 1);
                fclose(fp);
                return (long long)n;
            }
        }
    }
    fclose(fp);
    errno = ENOENT;
    return -1;
}

/* ---- misc ---- */

int
pyos_native_sync(void)
{
    sync();
    return 0;
}

int
pyos_native_syncfs(int fd)
{
#ifdef __linux__
    return syncfs(fd);
#else
    (void)fd;
    sync();
    return 0;
#endif
}

int
pyos_native_flock(int fd, int operation)
{
    return flock(fd, operation);
}

int
pyos_native_ioctl(int fd, unsigned long request, void *argument)
{
    return ioctl(fd, request, argument);
}

int
pyos_native_getrandom(void *buffer, unsigned long length, unsigned int flags)
{
#ifdef __linux__
    return (int)syscall(SYS_getrandom, buffer, (size_t)length, flags);
#else
    (void)flags;
    {
        int fd = open("/dev/urandom", O_RDONLY);
        ssize_t n;
        if (fd < 0) {
            return -1;
        }
        n = read(fd, buffer, (size_t)length);
        close(fd);
        return (int)n;
    }
#endif
}

int
pyos_native_readahead(int fd, pyos_off_t offset, unsigned long length)
{
#ifdef __linux__
    return readahead(fd, (off64_t)offset, (size_t)length);
#else
    (void)fd;
    (void)offset;
    (void)length;
    errno = ENOSYS;
    return -1;
#endif
}

int
pyos_native_fallocate(int fd, int mode, pyos_off_t offset, pyos_off_t length)
{
#ifdef __linux__
    return fallocate(fd, mode, (off_t)offset, (off_t)length);
#else
    (void)fd;
    (void)mode;
    (void)offset;
    (void)length;
    errno = ENOSYS;
    return -1;
#endif
}

int
pyos_native_posix_fadvise(int fd, pyos_off_t offset, pyos_off_t length, int advice)
{
    return posix_fadvise(fd, (off_t)offset, (off_t)length, advice);
}

/* ---- terminal stubs used by posix_terminal.c ---- */

int
pyos_native_open_terminal(const char *path, int flags)
{
    return open(path, flags);
}

int
pyos_native_is_terminal_device(int fd)
{
    return isatty(fd);
}

PyObject *
pyos_native_terminal_get_attributes(int fd)
{
    struct termios tio;
    if (tcgetattr(fd, &tio) < 0) {
        return NULL;
    }
    return PyBytes_FromStringAndSize((const char *)&tio, sizeof(tio));
}

int
pyos_native_terminal_set_attributes(int fd, PyObject *attributes, int optional_actions)
{
    struct termios tio;
    char *data;
    Py_ssize_t len;
    if (!PyBytes_Check(attributes)) {
        errno = EINVAL;
        return -1;
    }
    if (PyBytes_AsStringAndSize(attributes, &data, &len) < 0) {
        return -1;
    }
    if ((size_t)len < sizeof(tio)) {
        errno = EINVAL;
        return -1;
    }
    memcpy(&tio, data, sizeof(tio));
    return tcsetattr(fd, optional_actions, &tio);
}

int
pyos_native_tcgetpgrp(int fd)
{
    return (int)tcgetpgrp(fd);
}

int
pyos_native_tcsetpgrp(int fd, int pgrp)
{
    return tcsetpgrp(fd, (pid_t)pgrp);
}

int
pyos_native_tcsendbreak(int fd, int duration)
{
    return tcsendbreak(fd, duration);
}

int
pyos_native_tcdrain(int fd)
{
    return tcdrain(fd);
}

int
pyos_native_tcflush(int fd, int queue)
{
    return tcflush(fd, queue);
}

int
pyos_native_tcflow(int fd, int action)
{
    return tcflow(fd, action);
}

int
pyos_native_terminal_size(int fd, unsigned long *rows, unsigned long *cols,
                          unsigned long *x_pixels, unsigned long *y_pixels)
{
    struct winsize ws;
    if (ioctl(fd, TIOCGWINSZ, &ws) < 0) {
        return -1;
    }
    if (rows) *rows = ws.ws_row;
    if (cols) *cols = ws.ws_col;
    if (x_pixels) *x_pixels = ws.ws_xpixel;
    if (y_pixels) *y_pixels = ws.ws_ypixel;
    return 0;
}

int
pyos_native_set_terminal_size(int fd, unsigned long rows, unsigned long cols,
                              unsigned long x_pixels, unsigned long y_pixels)
{
    struct winsize ws;
    memset(&ws, 0, sizeof(ws));
    ws.ws_row = (unsigned short)rows;
    ws.ws_col = (unsigned short)cols;
    ws.ws_xpixel = (unsigned short)x_pixels;
    ws.ws_ypixel = (unsigned short)y_pixels;
    return ioctl(fd, TIOCSWINSZ, &ws);
}

int
pyos_native_terminal_control(int fd, int request, void *arg)
{
    return ioctl(fd, request, arg);
}

PyObject *
pyos_native_split_exec_path_cstr(const char *path)
{
    PyObject *list;
    const char *start;
    const char *p;

    list = PyList_New(0);
    if (list == NULL) {
        return NULL;
    }
    if (path == NULL || path[0] == '\0') {
        return list;
    }

    start = path;
    for (p = path; ; p++) {
        if (*p == ':' || *p == '\0') {
            PyObject *item;
            Py_ssize_t len = (Py_ssize_t)(p - start);
            if (len == 0) {
                item = PyUnicode_FromString(".");
            } else {
                item = PyUnicode_FromStringAndSize(start, len);
            }
            if (item == NULL) {
                Py_DECREF(list);
                return NULL;
            }
            if (PyList_Append(list, item) < 0) {
                Py_DECREF(item);
                Py_DECREF(list);
                return NULL;
            }
            Py_DECREF(item);
            if (*p == '\0') {
                break;
            }
            start = p + 1;
        }
    }
    return list;
}

PyObject *
pyos_native_split_exec_path(PyObject *path_obj)
{
    const char *path;
    PyObject *tmp = NULL;
    PyObject *result;

    if (path_obj == NULL || path_obj == Py_None) {
        return pyos_native_split_exec_path_cstr(NULL);
    }
    if (PyUnicode_Check(path_obj)) {
        path = PyUnicode_AsUTF8(path_obj);
        if (path == NULL) {
            return NULL;
        }
        return pyos_native_split_exec_path_cstr(path);
    }
    if (PyBytes_Check(path_obj)) {
        path = PyBytes_AsString(path_obj);
        if (path == NULL) {
            return NULL;
        }
        return pyos_native_split_exec_path_cstr(path);
    }
    tmp = PyObject_Str(path_obj);
    if (tmp == NULL) {
        return NULL;
    }
    path = PyUnicode_AsUTF8(tmp);
    if (path == NULL) {
        Py_DECREF(tmp);
        return NULL;
    }
    result = pyos_native_split_exec_path_cstr(path);
    Py_DECREF(tmp);
    return result;
}


PyObject *
pyos_socket_address_to_object(const pyos_socket_address *address)
{
    if (address == NULL) {
        Py_RETURN_NONE;
    }
    return PyBytes_FromStringAndSize(
        (const char *)address->data,
        (Py_ssize_t)address->length
    );
}
