from setuptools import setup, Extension, find_packages
from Cython.Build import cythonize
import numpy as np

ext_modules = [
    Extension(
        "lapcsa.core.lapcsa",
        sources=["lapcsa/core/lapcsa.pyx"],
        extra_compile_args=['-O3'],
        include_dirs=[np.get_include()],
        language='c'
    )
]

setup(
    name="lapcsa",
    version="0.1.0",
    author="Kyle McDonald",
    description="A fast implementation of the CSA algorithm",
    url="https://github.com/kylemcdonald/lapcsa",
    packages=find_packages(),
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ],
    python_requires=">=3.6",
    install_requires=[
        "numpy>=1.19.0",
        "scipy>=1.6.0",
    ],
    ext_modules=cythonize(ext_modules, language_level=3),
    include_package_data=True,
)