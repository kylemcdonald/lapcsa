# run:
# python setup.py build_ext --inplace
# python test.py

from distutils.core import setup, Extension
from Cython.Build import cythonize

example_module = Extension('lapcsa',
    sources=['lapcsa.pyx'],
    extra_compile_args=['-O3'],
    language='c')

setup(
    name='lapcsa',
    packages=['lapcsa'],
    ext_modules=cythonize(example_module, language_level=3)
)