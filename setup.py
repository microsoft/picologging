from skbuild import setup
from setuptools import find_packages


with open("./README.md", "r") as fh:
    long_description = fh.read()


setup(
    name="picologging",
    packages=find_packages(where="src"),
    package_dir={"": "src"},
    version="0.7.2",
    author="Microsoft",
    description="A fast and lightweight logging library for Python",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/microsoft/picologging",
    license="MIT License",
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",
        "Topic :: System :: Logging",
        "Programming Language :: Python :: Implementation :: CPython",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
    ],
    install_requires=[],
    python_requires=">=3.7",
    extras_require={
        "dev": [
            "rich",
            "pytest",
            "pytest-cov",
            "black",
        ]
    },
    project_urls={
        "Source": "https://github.com/microsoft/picologging",
        "Documentation": "https://microsoft.github.io/picologging/",
    },
)
