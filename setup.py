from setuptools import find_packages
from skbuild import setup

with open("./README.md") as fh:
    long_description = fh.read()


setup(
    name="picologging",
    packages=find_packages(where="src"),
    package_dir={"": "src"},
    package_data={
        "picologging": ["py.typed", "__init__.pyi", "config.pyi", "handlers.pyi"]
    },
    version="0.9.0",
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
        "Development Status :: 4 - Beta",
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
            "hypothesis",
            "flaky",
            "black",
            "pre-commit",
        ]
    },
    project_urls={
        "Source": "https://github.com/microsoft/picologging",
        "Documentation": "https://microsoft.github.io/picologging/",
    },
)
