from setuptools import setup, Extension, find_packages
from pathlib import Path
import platform


ROOT = Path(__file__).resolve().parent
SRC = ROOT / "src"
PYOS = SRC / "pyos"
NATIVE = PYOS / "native"
LINUX_DECODER = NATIVE / "decoders" / "linux"


POSIX_SOURCES = [
    "posix_core.c",
    "posix_directory.c",
    "posix_env.c",
    "posix_environment.c",
    "posix_exec.c",
    "posix_fd.c",
    "posix_file.c",
    "posix_fs.c",
    "posix_misc.c",
    "posix_mmap.c",
    "posix_module.c",
    "posix_mount.c",
    "posix_native_impl.c",
    "posix_poll.c",
    "posix_process.c",
    "posix_resource.c",
    "posix_signal.c",
    "posix_socket.c",
    "posix_stat.c",
    "posix_stat_decode.c",
    "posix_sysinfo.c",
    "posix_terminal.c",
    "posix_time.c",
    "posix_user.c",
]


LINUX_SOURCES = [
    "linux_decoder.c",
    "stat_aarch64.c",
    "stat_armv7.c",
    "stat_i386.c",
    "stat_ppc64le.c",
    "stat_riscv64.c",
    "stat_x84_64.c",
]


sources = [
    str(NATIVE / source)
    for source in POSIX_SOURCES
]


if platform.system() == "Linux":
    sources.extend(
        str(LINUX_DECODER / source)
        for source in LINUX_SOURCES
    )


native_extension = Extension(
    "pyos._native",
    sources=sources,
    include_dirs=[
        str(NATIVE),
        str(LINUX_DECODER),
    ],
    define_macros=[
        ("PY_SSIZE_T_CLEAN", "1"),
    ],
    extra_compile_args=[
        "-Wall",
        "-Wextra",
    ],
)


README = ROOT / "README.md"

long_description = (
    README.read_text(encoding="utf-8")
    if README.exists()
    else ""
)


setup(
    name="pyos",
    version="0.1.0",
    description="Native POSIX operating system interface for Python",
    long_description=long_description,
    long_description_content_type="text/markdown",
    package_dir={"": "src"},
    packages=find_packages("src"),
    ext_modules=[native_extension],
    include_package_data=True,
    python_requires=">=3.9",
)
