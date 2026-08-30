from setuptools import setup, Extension, find_packages
from pathlib import Path
import platform

ROOT = Path(__file__).resolve().parent
NATIVE = ROOT / "src" / "pyos" / "native"
LINUX = NATIVE / "decoders" / "linux"

sources = []
for f in sorted(NATIVE.glob("posix_*.c")):
    sources.append(f"src/pyos/native/{f.name}")
    print(f"Found: {f.name}")

if platform.system() == "Linux" and LINUX.exists():
    for f in sorted(LINUX.glob("*.c")):
        sources.append(f"src/pyos/native/decoders/linux/{f.name}")
        print(f"Found Linux: {f.name}")

print(f"Total C files: {len(sources)}")

ext = Extension(
    "pyos._native",
    sources=sources,
    include_dirs=[
        "src/pyos/native",
        "src/pyos/native/decoders/linux",
    ],
    extra_compile_args=["-O3"],
)

long_description = (ROOT / "README.md").read_text(encoding="utf-8") if (ROOT / "README.md").exists() else ""

setup(
    name="pyos-dan",
    version="0.1.1",
    description="Native POSIX operating system interface for Python",
    long_description=long_description,
    long_description_content_type="text/markdown",
    package_dir={"": "src"},
    packages=find_packages(where="src"),
    ext_modules=[ext],
    include_package_data=True,
    python_requires=">=3.9",
)
