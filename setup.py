from setuptools import find_packages, setup, Extension
import pybind11
import os
import numpy as np

if os.environ.get("READTHEDOCS") == "True":
    extra_compile_args=['-std=c++2a'] 
else:
    extra_compile_args=["/std:c++20"] if os.name == "nt" else ["-std=c++20"]
    

extension = Extension(
    "stampdb._backend._types",
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
    extra_compile_args=extra_compile_args,
)


def read_requirements():
    try:
        with open("requirements.txt", "r", encoding="utf-8") as f:
            return f.read().splitlines()
    except FileNotFoundError:
        return []


def read_long_description():
    try:
        with open("README.md", "r", encoding="utf-8") as f:
            return f.read()
    except FileNotFoundError:
        return "A tiny C++ Time Series Database library designed for compatibility with the PyData Ecosystem."


setup(
    name="stampdb",
    version="0.1.0",
    description="A tiny C++ Time Series Database library designed for compatibility with the PyData Ecosystem.",
    long_description=read_long_description(),
    long_description_content_type="text/markdown",
    author="Aadya A. Chinubhai",
    author_email="aadyachinubhai@gmail.com",
    url="https://github.com/aadya940/stampdb",
    ext_modules=[extension],
    packages=find_packages(),
    install_requires=read_requirements(),
    zip_safe=False,
    python_requires=">=3.7",
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",
        "Programming Language :: Python :: 3",
        "Programming Language :: C++",
    ],
)
