from setuptools import setup, Extension
import pybind11
import os
from setuptools.command.build_ext import build_ext
import numpy as np


class CustomBuildExt(build_ext):
    def build_extension(self, ext):
        # Set a custom output directory for the extension
        build_lib = os.path.join("stampdb", "_backend")
        os.makedirs(build_lib, exist_ok=True)
        self.build_lib = build_lib
        super().build_extension(ext)


extension = Extension(
    "_types",
    sources=[
        "stampdb/_backend/types.cpp",
        "src/csvparse.cpp",
        "src/appendonly.cpp",
        "src/algorithms.cpp",
        "src/fileio.cpp",
        "src/stampdb.cpp",
    ],
    include_dirs=[
        "include",
        "include/internal",
        "libs/csv2/include",
        pybind11.get_include(),
        os.path.join(pybind11.get_include(), "pybind11"),
        os.path.join(pybind11.get_include(), "numpy"),
        np.get_include(),
    ],
    language="c++",
    extra_compile_args=["/std:c++20"] if os.name == "nt" else ["-std=c++20"],
)

setup(
    name="stampdb",
    version="0.1.0",
    ext_modules=[extension],
    packages=["stampdb"],
    package_dir={"": "."},
    cmdclass={"build_ext": CustomBuildExt},
    install_requires=open("requirements.txt").read().splitlines(),
)
