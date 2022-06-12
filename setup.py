from skbuild import setup
from setuptools import find_packages


with open("./README.md", "r") as fh:
    long_description = fh.read()


setup(
    name="picologging",
    packages=find_packages(where="src"),
    package_dir={'': 'src'},
    version="0.1.0",
    author="Microsoft",
    description="A fast and lightweight logging library for Python",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/microsoft/picologging",
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ],
    install_requires=[],
    python_requires='>=3.7',
    extras_require = {
        "dev": [
            "rich",
            "pytest",
            "pytest-cov",
        ]
    }
)