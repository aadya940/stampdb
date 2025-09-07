from setuptools import setup, Extension
import pybind11
import os

# Get the absolute path of the current directory
current_dir = os.path.abspath(os.path.dirname(__file__))

# Define the extension module
extension = Extension(
    '_backend',
    sources=[
        'python/_backend.py/types.cpp',
        'src/csvparse.cpp',
        'src/appendonly.cpp',
        'src/atomicity.cpp',
        'src/algorithms.cpp',
        'src/fileio.cpp',
        'src/stampdb.cpp'
    ],
    include_dirs=[
        'include',
        'include/internal',
        'libs/csv2/include',
        pybind11.get_include(),
        os.path.join(pybind11.get_include(), 'pybind11')
    ],
    language='c++',
    extra_compile_args=['/std:c++20'] if os.name == 'nt' else ['-std=c++20']
)

setup(
    name="stampdb",
    version="0.1.0",
    ext_modules=[extension],
    packages=['python'],
    package_dir={'': '.'},
)
