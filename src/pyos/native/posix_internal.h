#ifndef PYOS_POSIX_INTERNAL_H
#define PYOS_POSIX_INTERNAL_H

#include <Python.h>

/* Linux sys/stat.h macros conflict with our field names. */
#ifdef st_atime
#undef st_atime
#endif
#ifdef st_mtime
#undef st_mtime
#endif
#ifdef st_ctime
#undef st_ctime
#endif

typedef signed char pyos_int8;
typedef unsigned char pyos_uint8;
typedef short pyos_int16;
typedef unsigned short pyos_uint16;
typedef int pyos_int32;
typedef unsigned int pyos_uint32;
typedef long long pyos_int64;
typedef unsigned long long pyos_uint64;

typedef int pyos_fd_t;
typedef long long pyos_pid_t;

typedef unsigned int pyos_mode_t;
typedef unsigned int pyos_uid_t;
typedef unsigned int pyos_gid_t;

typedef long long pyos_off_t;
typedef long long pyos_time_t;

typedef unsigned long long pyos_ino_t;
typedef unsigned long long pyos_dev_t;
typedef unsigned long long pyos_nlink_t;

typedef struct {
    pyos_mode_t st_mode;
    pyos_ino_t st_ino;
    pyos_dev_t st_dev;
    pyos_nlink_t st_nlink;
    pyos_uid_t st_uid;
    pyos_gid_t st_gid;
    pyos_off_t st_size;
    pyos_time_t st_atime;
    pyos_time_t st_mtime;
    pyos_time_t st_ctime;
} pyos_stat_result;

typedef struct {
    long tv_sec;
    long tv_nsec;
} pyos_timespec;

typedef struct {
    char *data;
    long long length;
    int owns_data;
    PyObject *owner;
} pyos_path_buffer;

typedef struct {
    int code;
    const char *name;
    const char *message;
} pyos_errno_info;

typedef struct {
    int fd;
    int flags;
    int valid;
} pyos_fd_info;

typedef struct {
    pyos_pid_t pid;
    int status;
    int exited;
    int signaled;
    int stopped;
    int continued;
    int exit_code;
    int signal_number;
} pyos_wait_result;

typedef struct {
    char *name;
    pyos_ino_t inode;
    pyos_off_t size;
    pyos_mode_t mode;
    int type;
} pyos_dir_entry;

typedef struct {
    pyos_int64 total;
    pyos_int64 free;
    pyos_int64 available;
    pyos_int64 used;
    pyos_int64 buffers;
    pyos_int64 cached;
    pyos_int64 swap_total;
    pyos_int64 swap_free;
} pyos_memory_info;

typedef struct {
    pyos_int64 user_seconds;
    pyos_int64 user_microseconds;
    pyos_int64 system_seconds;
    pyos_int64 system_microseconds;
    pyos_int64 max_rss;
    pyos_int64 minor_faults;
    pyos_int64 major_faults;
    pyos_int64 input_blocks;
    pyos_int64 output_blocks;
    pyos_int64 signals;
} pyos_resource_usage;

typedef struct {
    pyos_int64 load_1;
    pyos_int64 load_5;
    pyos_int64 load_15;
    pyos_int64 processes;
    pyos_int64 uptime;
} pyos_system_info;

typedef struct {
    pyos_int64 address;
    pyos_int64 length;
    int protection;
    int flags;
    pyos_fd_t fd;
    pyos_off_t offset;
} pyos_mmap_info;

typedef struct {
    pyos_fd_t fd;
    int events;
    int revents;
} pyos_poll_result;

typedef struct {
    unsigned int events;
    unsigned long long data;
} pyos_epoll_event;

typedef struct {
    int domain;
    int type;
    int protocol;
    pyos_fd_t fd;
} pyos_socket_info;

typedef struct {
    int family;
    unsigned long length;
    unsigned char data[128];
} pyos_socket_address;

PyObject *pyos_socket_address_to_object(const pyos_socket_address *address);

typedef struct {
    pyos_uid_t uid;
    pyos_gid_t gid;
    char *name;
    char *home;
    char *shell;
} pyos_user_info;

typedef struct {
    char *source;
    char *target;
    char *filesystem;
    char *options;
    unsigned long flags;
} pyos_mount_info;

#define PYOS_PATH_MAX 4096
#define PYOS_NAME_MAX 255

#define PYOS_NATIVE_STAT_SIZE 512

typedef unsigned char pyos_native_stat_buffer[
    PYOS_NATIVE_STAT_SIZE
];

#define PYOS_S_IFMT 0170000
#define PYOS_S_IFREG 0100000
#define PYOS_S_IFDIR 0040000
#define PYOS_S_IFLNK 0120000
#define PYOS_S_IFCHR 0020000
#define PYOS_S_IFBLK 0060000
#define PYOS_S_IFIFO 0010000
#define PYOS_S_IFSOCK 0140000

#define PYOS_O_RDONLY 00000000
#define PYOS_O_WRONLY 00000001
#define PYOS_O_RDWR 00000002
#define PYOS_O_CREAT 00000100
#define PYOS_O_EXCL 00000200
#define PYOS_O_TRUNC 00001000
#define PYOS_O_APPEND 00002000
#define PYOS_O_NONBLOCK 00004000
#define PYOS_O_DSYNC 00010000
#define PYOS_O_DIRECT 00040000
#define PYOS_O_LARGEFILE 00100000
#define PYOS_O_DIRECTORY 00200000
#define PYOS_O_NOFOLLOW 00400000
#define PYOS_O_CLOEXEC 02000000
#define PYOS_O_SYNC 04010000

#define PYOS_SEEK_SET 0
#define PYOS_SEEK_CUR 1
#define PYOS_SEEK_END 2

#define PYOS_F_OK 0
#define PYOS_X_OK 1
#define PYOS_W_OK 2
#define PYOS_R_OK 4

#define PYOS_STDIN_FILENO 0
#define PYOS_STDOUT_FILENO 1
#define PYOS_STDERR_FILENO 2

#define PYOS_AT_FDCWD (-100)

#define PYOS_AT_SYMLINK_NOFOLLOW 0x100
#define PYOS_AT_REMOVEDIR 0x200
#define PYOS_AT_SYMLINK_FOLLOW 0x400
#define PYOS_AT_NO_AUTOMOUNT 0x800
#define PYOS_AT_EMPTY_PATH 0x1000

#define PYOS_DT_UNKNOWN 0
#define PYOS_DT_FIFO 1
#define PYOS_DT_CHR 2
#define PYOS_DT_DIR 4
#define PYOS_DT_BLK 6
#define PYOS_DT_REG 8
#define PYOS_DT_LNK 10
#define PYOS_DT_SOCK 12

#define PYOS_WNOHANG 1
#define PYOS_WUNTRACED 2
#define PYOS_WCONTINUED 8

#define PYOS_SIGHUP 1
#define PYOS_SIGINT 2
#define PYOS_SIGQUIT 3
#define PYOS_SIGILL 4
#define PYOS_SIGTRAP 5
#define PYOS_SIGABRT 6
#define PYOS_SIGBUS 7
#define PYOS_SIGFPE 8
#define PYOS_SIGKILL 9
#define PYOS_SIGUSR1 10
#define PYOS_SIGSEGV 11
#define PYOS_SIGUSR2 12
#define PYOS_SIGPIPE 13
#define PYOS_SIGALRM 14
#define PYOS_SIGTERM 15
#define PYOS_SIGCHLD 17
#define PYOS_SIGCONT 18
#define PYOS_SIGSTOP 19
#define PYOS_SIGTSTP 20
#define PYOS_SIGTTIN 21
#define PYOS_SIGTTOU 22

#define PYOS_EPERM 1
#define PYOS_ENOENT 2
#define PYOS_ESRCH 3
#define PYOS_EINTR 4
#define PYOS_EIO 5
#define PYOS_ENXIO 6
#define PYOS_E2BIG 7
#define PYOS_ENOEXEC 8
#define PYOS_EBADF 9
#define PYOS_ECHILD 10
#define PYOS_EAGAIN 11
#define PYOS_ENOMEM 12
#define PYOS_EACCES 13
#define PYOS_EFAULT 14
#define PYOS_EBUSY 16
#define PYOS_EEXIST 17
#define PYOS_EXDEV 18
#define PYOS_ENODEV 19
#define PYOS_ENOTDIR 20
#define PYOS_EISDIR 21
#define PYOS_EINVAL 22
#define PYOS_ENFILE 23
#define PYOS_EMFILE 24
#define PYOS_ENOTTY 25
#define PYOS_ETXTBSY 26
#define PYOS_EFBIG 27
#define PYOS_ENOSPC 28
#define PYOS_ESPIPE 29
#define PYOS_EROFS 30
#define PYOS_EMLINK 31
#define PYOS_EPIPE 32
#define PYOS_EDOM 33
#define PYOS_ERANGE 34
#define PYOS_ENAMETOOLONG 36
#define PYOS_ENOSYS 38
#define PYOS_ENOTEMPTY 39
#define PYOS_ELOOP 40

typedef struct {
    int (*getcwd)(char *, unsigned long);
    int (*chdir)(const char *);
    int (*fchdir)(int);

    int (*mkdir)(const char *, pyos_mode_t);
    int (*rmdir)(const char *);
    int (*unlink)(const char *);
    int (*rename)(const char *, const char *);

    int (*open)(const char *, int, pyos_mode_t);
    int (*close)(int);

    long long (*read)(
        int,
        void *,
        unsigned long
    );

    long long (*write)(
        int,
        const void *,
        unsigned long
    );

    long long (*pread)(
        int,
        void *,
        unsigned long,
        pyos_off_t
    );

    long long (*pwrite)(
        int,
        const void *,
        unsigned long,
        pyos_off_t
    );

    pyos_off_t (*lseek)(
        int,
        pyos_off_t,
        int
    );

    int (*fsync)(int);
    int (*fdatasync)(int);

    int (*ftruncate)(
        int,
        pyos_off_t
    );

    int (*stat)(
        const char *,
        pyos_stat_result *
    );

    int (*lstat)(
        const char *,
        pyos_stat_result *
    );

    int (*fstat)(
        int,
        pyos_stat_result *
    );

    int (*chmod)(
        const char *,
        pyos_mode_t
    );

    int (*fchmod)(
        int,
        pyos_mode_t
    );

    int (*chown)(
        const char *,
        pyos_uid_t,
        pyos_gid_t
    );

    int (*fchown)(
        int,
        pyos_uid_t,
        pyos_gid_t
    );

    int (*lchown)(
        const char *,
        pyos_uid_t,
        pyos_gid_t
    );

    int (*access)(
        const char *,
        int
    );

    int (*truncate)(
        const char *,
        pyos_off_t
    );

    int (*dup)(int);

    int (*dup2)(
        int,
        int
    );

    int (*dup3)(
        int,
        int,
        int
    );

    int (*pipe)(
        int[2]
    );

    int (*pipe2)(
        int[2],
        int
    );

    int (*isatty)(int);

    int (*ttyname)(
        int,
        char *,
        unsigned long
    );

    int (*symlink)(
        const char *,
        const char *
    );

    int (*link)(
        const char *,
        const char *
    );

    long long (*readlink)(
        const char *,
        char *,
        unsigned long
    );

    int (*fsync_dir)(int);

    pyos_pid_t (*getpid)(void);
    pyos_pid_t (*getppid)(void);

    pyos_uid_t (*getuid)(void);
    pyos_uid_t (*geteuid)(void);

    pyos_gid_t (*getgid)(void);
    pyos_gid_t (*getegid)(void);

    long (*getgroups)(
        long,
        pyos_gid_t *
    );

    int (*setuid)(
        pyos_uid_t
    );

    int (*setgid)(
        pyos_gid_t
    );

    int (*setpgid)(
        pyos_pid_t,
        pyos_pid_t
    );

    pyos_pid_t (*getpgid)(
        pyos_pid_t
    );

    pyos_pid_t (*getpgrp)(void);
    pyos_pid_t (*setsid)(void);

    pyos_pid_t (*getsid)(
        pyos_pid_t
    );

    pyos_pid_t (*fork)(void);
    pyos_pid_t (*vfork)(void);

    int (*waitpid)(
        pyos_pid_t,
        int,
        pyos_wait_result *
    );

    void (*exit)(
        int
    );

    int (*kill)(
        pyos_pid_t,
        int
    );

    int (*pause)(void);

    unsigned long (*sleep)(
        unsigned long
    );

    int (*usleep)(
        unsigned long
    );

    int (*gethostname)(
        char *,
        unsigned long
    );

    const char *(*getlogin)(void);

    int (*clock_gettime)(
        int,
        pyos_timespec *
    );

    int (*clock_getres)(
        int,
        pyos_timespec *
    );

    int (*nanosleep)(
        const pyos_timespec *,
        pyos_timespec *
    );

    int (*poll)(
        pyos_poll_result *,
        unsigned long,
        int
    );

    int (*socket)(
        int,
        int,
        int
    );

    int (*socketpair)(
        int,
        int,
        int,
        int[2]
    );

    int (*bind)(
        int,
        const void *,
        unsigned long
    );

    int (*connect)(
        int,
        const void *,
        unsigned long
    );

    int (*listen)(
        int,
        int
    );

    int (*accept)(
        int,
        void *,
        unsigned long *
    );

    long long (*send)(
        int,
        const void *,
        unsigned long,
        int
    );

    long long (*recv)(
        int,
        void *,
        unsigned long,
        int
    );

    int (*shutdown)(
        int,
        int
    );

    int (*getsockopt)(
        int,
        int,
        int,
        void *,
        unsigned long *
    );

    int (*setsockopt)(
        int,
        int,
        int,
        const void *,
        unsigned long
    );

    int (*getpeername)(
        int,
        void *,
        unsigned long *
    );

    int (*getsockname)(
        int,
        void *,
        unsigned long *
    );

    void *(*mmap)(
        void *,
        unsigned long,
        int,
        int,
        int,
        pyos_off_t
    );

    int (*munmap)(
        void *,
        unsigned long
    );

    int (*mprotect)(
        void *,
        unsigned long,
        int
    );

    int (*msync)(
        void *,
        unsigned long,
        int
    );

    int (*madvise)(
        void *,
        unsigned long,
        int
    );

    int (*mlock)(
        const void *,
        unsigned long
    );

    int (*munlock)(
        const void *,
        unsigned long
    );

    int (*get_system_info)(
        pyos_system_info *
    );

    int (*get_memory_info)(
        pyos_memory_info *
    );

    pyos_int64 (*uptime)(void);

    int (*execve)(
        const char *,
        char *const[],
        char *const[]
    );

    int (*execv)(
        const char *,
        char *const[]
    );

    int (*execvp)(
        const char *,
        char *const[]
    );

    int (*execvpe)(
        const char *,
        char *const[],
        char *const[]
    );

    int (*get_user)(
        pyos_uid_t,
        pyos_user_info *
    );

    int (*get_group)(
        pyos_gid_t,
        pyos_user_info *
    );

    int (*get_current_user)(
        pyos_user_info *
    );

    void (*free_user_info)(
        pyos_user_info *
    );

    int (*mount)(
        const char *,
        const char *,
        const char *,
        unsigned long,
        const char *
    );

    int (*umount)(
        const char *
    );

    int (*remount)(
        const char *,
        unsigned long,
        const char *
    );

    int (*sync)(void);

    int (*syncfs)(int);

    int (*flock)(
        int,
        int
    );

    int (*ioctl)(
        int,
        unsigned long,
        void *
    );

    int (*getpriority)(
        int,
        pyos_pid_t
    );

    int (*setpriority)(
        int,
        pyos_pid_t,
        int
    );

    int (*sched_yield)(void);

    int (*sched_getcpu)(void);

    int (*getrandom)(
        void *,
        unsigned long,
        unsigned int
    );

    int (*readahead)(
        int,
        pyos_off_t,
        unsigned long
    );

    int (*fallocate)(
        int,
        int,
        pyos_off_t,
        pyos_off_t
    );

    int (*posix_fadvise)(
        int,
        pyos_off_t,
        pyos_off_t,
        int
    );

} pyos_native_api;

extern PyObject *pyos_posix_error;
extern PyObject *pyos_native_error;

extern PyMethodDef *pyos_get_core_methods(void);
extern PyMethodDef *pyos_get_fd_methods(void);
extern PyMethodDef *pyos_get_fs_methods(void);
extern PyMethodDef *pyos_get_directory_methods(void);
extern PyMethodDef *pyos_get_process_methods(void);
extern PyMethodDef *pyos_get_time_methods(void);
extern PyMethodDef *pyos_get_signal_methods(void);
extern PyMethodDef *pyos_get_terminal_methods(void);
extern PyMethodDef *pyos_get_poll_methods(void);
extern PyMethodDef *pyos_get_socket_methods(void);
extern PyMethodDef *pyos_get_mmap_methods(void);
extern PyMethodDef *pyos_get_sysinfo_methods(void);
extern PyMethodDef *pyos_get_exec_methods(void);
extern PyMethodDef *pyos_get_user_methods(void);
extern PyMethodDef *pyos_get_mount_methods(void);
extern PyMethodDef *pyos_get_misc_methods(void);
extern PyMethodDef *pyos_get_env_methods(void);
extern PyMethodDef *pyos_get_file_methods(void);
extern PyMethodDef *pyos_get_resource_methods(void);

int pyos_initialize_native(void);
void pyos_finalize_native(void);

int pyos_path_from_object(
    PyObject *,
    pyos_path_buffer *
);

void pyos_path_release(
    pyos_path_buffer *
);

int pyos_path_validate(
    const pyos_path_buffer *
);

int pyos_get_errno(void);

const char *pyos_native_strerror(
    int
);

PyObject *pyos_raise_errno(void);

PyObject *pyos_raise_errno_with_path(
    const char *
);

PyObject *pyos_raise_native_error(
    const char *
);

PyObject *pyos_make_stat_dict(
    const pyos_stat_result *
);

int pyos_decode_stat(
    const void *,
    pyos_stat_result *
);

int pyos_mode_is_regular(
    pyos_mode_t
);

int pyos_mode_is_directory(
    pyos_mode_t
);

int pyos_mode_is_symlink(
    pyos_mode_t
);

int pyos_mode_is_character_device(
    pyos_mode_t
);

int pyos_mode_is_block_device(
    pyos_mode_t
);

int pyos_mode_is_fifo(
    pyos_mode_t
);

int pyos_mode_is_socket(
    pyos_mode_t
);

PyObject *pyos_bool_result(
    int
);

PyObject *pyos_bytes_from_buffer(
    const char *,
    long long
);

PyObject *pyos_unicode_from_buffer(
    const char *,
    long long
);

PyObject *pyos_path_result(
    PyObject *,
    const char *,
    long long
);

int pyos_require_fd(
    PyObject *,
    pyos_fd_t *
);

int pyos_require_integer(
    PyObject *,
    const char *,
    long long *
);

int pyos_native_open(
    const char *,
    int,
    pyos_mode_t
);

int pyos_native_close(
    int
);

long long pyos_native_read(
    int,
    void *,
    unsigned long
);

long long pyos_native_write(
    int,
    const void *,
    unsigned long
);

long long pyos_native_pread(
    int,
    void *,
    unsigned long,
    pyos_off_t
);

long long pyos_native_pwrite(
    int,
    const void *,
    unsigned long,
    pyos_off_t
);

pyos_off_t pyos_native_lseek(
    int,
    pyos_off_t,
    int
);

int pyos_native_fsync(
    int
);

int pyos_native_fdatasync(
    int
);

int pyos_native_ftruncate(
    int,
    pyos_off_t
);

int pyos_native_stat(
    const char *,
    pyos_stat_result *
);

int pyos_native_lstat(
    const char *,
    pyos_stat_result *
);

int pyos_native_fstat(
    int,
    pyos_stat_result *
);

int pyos_native_mkdir(
    const char *,
    pyos_mode_t
);

int pyos_native_rmdir(
    const char *
);

int pyos_native_unlink(
    const char *
);

int pyos_native_rename(
    const char *,
    const char *
);

int pyos_native_chdir(
    const char *
);

int pyos_native_fchdir(
    int
);

int pyos_native_getcwd(
    char *,
    unsigned long
);

int pyos_native_chmod(
    const char *,
    pyos_mode_t
);

int pyos_native_fchmod(
    int,
    pyos_mode_t
);

int pyos_native_chown(
    const char *,
    pyos_uid_t,
    pyos_gid_t
);

int pyos_native_fchown(
    int,
    pyos_uid_t,
    pyos_gid_t
);

int pyos_native_lchown(
    const char *,
    pyos_uid_t,
    pyos_gid_t
);

int pyos_native_access(
    const char *,
    int
);

int pyos_native_truncate(
    const char *,
    pyos_off_t
);

int pyos_native_dup(
    int
);

int pyos_native_dup2(
    int,
    int
);

int pyos_native_dup3(
    int,
    int,
    int
);

int pyos_native_pipe(
    int[2]
);

int pyos_native_pipe2(
    int[2],
    int
);

int pyos_native_isatty(
    int
);

int pyos_native_ttyname(
    int,
    char *,
    unsigned long
);

int pyos_native_symlink(
    const char *,
    const char *
);

int pyos_native_link(
    const char *,
    const char *
);

long long pyos_native_readlink(
    const char *,
    char *,
    unsigned long
);

int pyos_native_fsync_dir(
    int
);

pyos_pid_t pyos_native_getpid(void);
pyos_pid_t pyos_native_getppid(void);

pyos_uid_t pyos_native_getuid(void);
pyos_uid_t pyos_native_geteuid(void);

pyos_gid_t pyos_native_getgid(void);
pyos_gid_t pyos_native_getegid(void);

long pyos_native_getgroups(
    long,
    pyos_gid_t *
);

int pyos_native_setuid(
    pyos_uid_t
);

int pyos_native_setgid(
    pyos_gid_t
);

int pyos_native_setpgid(
    pyos_pid_t,
    pyos_pid_t
);

pyos_pid_t pyos_native_getpgid(
    pyos_pid_t
);

pyos_pid_t pyos_native_getpgrp(void);
pyos_pid_t pyos_native_setsid(void);

pyos_pid_t pyos_native_getsid(
    pyos_pid_t
);

pyos_pid_t pyos_native_fork(void);
pyos_pid_t pyos_native_vfork(void);

int pyos_native_waitpid(
    pyos_pid_t,
    int,
    pyos_wait_result *
);

void pyos_native_exit(
    int
);

int pyos_native_kill(
    pyos_pid_t,
    int
);

int pyos_native_pause(void);

unsigned long pyos_native_sleep(
    unsigned long
);

int pyos_native_usleep(
    unsigned long
);

int pyos_native_gethostname(
    char *,
    unsigned long
);

const char *pyos_native_getlogin(void);

int pyos_native_clock_gettime(
    int,
    pyos_timespec *
);

int pyos_native_clock_getres(
    int,
    pyos_timespec *
);

int pyos_native_nanosleep(
    const pyos_timespec *,
    pyos_timespec *
);

int pyos_native_poll(
    pyos_poll_result *,
    unsigned long,
    int
);

int pyos_native_socket(
    int,
    int,
    int
);

int pyos_native_socketpair(
    int,
    int,
    int,
    int[2]
);

int pyos_native_bind(
    int,
    const void *,
    unsigned long
);

int pyos_native_connect(
    int,
    const void *,
    unsigned long
);

int pyos_native_listen(
    int,
    int
);

int pyos_native_accept(int fd, pyos_socket_address *address);

long long pyos_native_send(
    int,
    const void *,
    unsigned long,
    int
);

long long pyos_native_recv(
    int,
    void *,
    unsigned long,
    int
);

int pyos_native_shutdown(
    int,
    int
);

int pyos_native_getsockopt(
    int,
    int,
    int,
    void *,
    unsigned long *
);

int pyos_native_setsockopt(
    int,
    int,
    int,
    const void *,
    unsigned long
);

int pyos_native_getpeername(int fd, pyos_socket_address *address);

int pyos_native_getsockname(int fd, pyos_socket_address *address);

void *pyos_native_mmap(
    void *,
    unsigned long,
    int,
    int,
    int,
    pyos_off_t
);

int pyos_native_munmap(
    void *,
    unsigned long
);

int pyos_native_mprotect(
    void *,
    unsigned long,
    int
);

int pyos_native_msync(
    void *,
    unsigned long,
    int
);

int pyos_native_madvise(
    void *,
    unsigned long,
    int
);

int pyos_native_mlock(
    const void *,
    unsigned long
);

int pyos_native_munlock(
    const void *,
    unsigned long
);

int pyos_native_get_system_info(
    pyos_system_info *
);

int pyos_native_get_memory_info(
    unsigned long long *total,
    unsigned long long *available
);
int pyos_native_get_memory_info_struct(
    pyos_memory_info *
);

pyos_int64 pyos_native_uptime(void);

int pyos_native_execve(
    const char *path,
    char *const argv[],
    char *const envp[]
);

int pyos_native_execvp(
    const char *file,
    char *const argv[]
);

int pyos_native_execvpe(
    const char *file,
    char *const argv[],
    char *const envp[]
);

int pyos_native_get_user(
    pyos_uint32 uid,
    pyos_user_info *result
);

int pyos_native_get_group(
    pyos_uint32 gid,
    pyos_user_info *result
);

int pyos_native_get_current_user(
    pyos_user_info *result
);

void pyos_native_free_user_info(
    pyos_user_info *info
);

int pyos_native_mount(
    const char *source,
    const char *target,
    const char *filesystem,
    unsigned long flags,
    const char *options
);

int pyos_native_umount(
    const char *target,
    unsigned long flags
);

int pyos_native_remount(
    const char *target,
    unsigned long flags,
    const char *options
);

int pyos_native_sync(void);

int pyos_native_syncfs(
    int fd
);

int pyos_native_flock(
    int fd,
    int operation
);

int pyos_native_ioctl(
    int fd,
    unsigned long request,
    void *argument
);

int pyos_native_getpriority(
    int which,
    pyos_int64 who
);

int pyos_native_setpriority(
    int which,
    pyos_int64 who,
    int priority
);

int pyos_native_sched_yield(void);

int pyos_native_sched_getcpu(void);

int pyos_native_getrandom(
    void *buffer,
    unsigned long length,
    unsigned int flags
);

int pyos_native_readahead(
    int fd,
    pyos_off_t offset,
    unsigned long length
);

int pyos_native_fallocate(
    int fd,
    int mode,
    pyos_off_t offset,
    pyos_off_t length
);

int pyos_native_posix_fadvise(
    int fd,
    pyos_off_t offset,
    pyos_off_t length,
    int advice
);

int pyos_native_fsync_dir(
    int fd
);


/* Additional native entry points used by higher-level modules. */
int pyos_native_errno(void);
const char *pyos_native_getenv(const char *name);
int pyos_native_setenv(const char *name, const char *value, int overwrite);
int pyos_native_unsetenv(const char *name);
int pyos_native_putenv(char *string);
int pyos_native_clearenv(void);
char **pyos_native_environ(void);
int pyos_native_umask(int mask);
int pyos_native_getumask(void);
int pyos_native_getdomainname(char *buffer, unsigned long length);
long long pyos_native_opendir(const char *path);
int pyos_native_closedir(long long handle);
long long pyos_native_readdir(long long handle, void *buffer, unsigned long size);
int pyos_native_dirfd(long long handle);
int pyos_native_getdents(int fd, void *buffer, unsigned long size);
int pyos_native_fstatat(int dirfd, const char *path, pyos_stat_result *result, int flags);
int pyos_native_mkdirat(int dirfd, const char *path, pyos_mode_t mode);
int pyos_native_unlinkat(int dirfd, const char *path, int flags);
int pyos_native_renameat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath);
int pyos_native_setgroups(long size, const pyos_gid_t *list);
int pyos_native_killpg(pyos_pid_t pgrp, int sig);
int pyos_native_execv(const char *path, char *const argv[]);
PyObject *pyos_native_get_exec_path(PyObject *environment);
PyObject *pyos_native_split_exec_path(PyObject *path_obj);
PyObject *pyos_native_split_exec_path_cstr(const char *path);
int pyos_native_monotonic(pyos_timespec *ts);
int pyos_native_realtime(pyos_timespec *ts);
int pyos_native_process_time(pyos_timespec *ts);
int pyos_native_thread_time(pyos_timespec *ts);
int pyos_native_alarm(unsigned int seconds);
int pyos_native_raise(int sig);
PyObject *pyos_native_sigaction(int signal_number, PyObject *handler, unsigned long long flags, unsigned long long mask);
int pyos_native_block_signals(unsigned long long mask);
int pyos_native_unblock_signals(unsigned long long mask);
int pyos_native_set_signal_mask(unsigned long long mask);
int pyos_native_get_signal_mask(void *set);
int pyos_native_pending_signals(void *set);
int pyos_native_signal_default(int sig);
int pyos_native_signal_ignore(int sig);
int pyos_native_signal_set_handler(int sig, void (*handler)(int));
int pyos_native_queue_signal(pyos_pid_t pid, int sig, const void *value);
int pyos_native_wait_signal(unsigned long long mask, int *signal_number);
int pyos_native_wait_signal_timeout(unsigned long long mask, long seconds, long nanoseconds, int *signal_number);
int pyos_native_getitimer(int which, long long *interval_seconds, long *interval_microseconds, long long *value_seconds, long *value_microseconds);
int pyos_native_setitimer(int which, double value, double interval, double *previous_value, double *previous_interval);
PyObject *pyos_native_select(PyObject *read_objects, PyObject *write_objects, PyObject *error_objects, double timeout);
int pyos_native_epoll_create(int size);
int pyos_native_epoll_ctl(int epfd, int op, int fd, unsigned int events, unsigned long long data);
int pyos_native_epoll_wait(int epfd, pyos_epoll_event *events, int maxevents, int timeout);
int pyos_native_sysinfo(void *info);
int pyos_native_getloadavg(double loadavg[], int nelem);
int pyos_native_getpagesize(void);
int pyos_native_getrlimit(int resource, unsigned long long *soft, unsigned long long *hard);
int pyos_native_setrlimit(int resource, unsigned long long soft, unsigned long long hard);
int pyos_native_getrusage(int who, pyos_resource_usage *usage);
int pyos_native_nice(int inc);
int pyos_native_resource_count(void);
int pyos_native_bind_mount(const char *source, const char *target, unsigned long flags);
int pyos_native_move_mount(const char *source, const char *target, unsigned long flags);
int pyos_native_mount_readonly(const char *target, unsigned long flags);
int pyos_native_is_mounted(const char *target);
long long pyos_native_get_mount_type(const char *target, char *buffer, unsigned long length);
int pyos_native_open_terminal(const char *path, int flags);
int pyos_native_is_terminal_device(int fd);
PyObject *pyos_native_terminal_get_attributes(int fd);
int pyos_native_terminal_set_attributes(int fd, PyObject *attributes, int optional_actions);
int pyos_native_tcgetpgrp(int fd);
int pyos_native_tcsetpgrp(int fd, int pgrp);
int pyos_native_tcsendbreak(int fd, int duration);
int pyos_native_tcdrain(int fd);
int pyos_native_tcflush(int fd, int queue);
int pyos_native_tcflow(int fd, int action);
int pyos_native_terminal_size(int fd, unsigned long *rows, unsigned long *cols, unsigned long *x_pixels, unsigned long *y_pixels);
int pyos_native_set_terminal_size(int fd, unsigned long rows, unsigned long cols, unsigned long x_pixels, unsigned long y_pixels);
int pyos_native_terminal_control(int fd, int request, void *arg);


#endif
