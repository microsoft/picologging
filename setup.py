from skbuild import setup
import picologging


with open("./README.md", "r") as fh:
    long_description = fh.read()


setup(
    name="picologging",
    packages=["picologging"],
    version=picologging.__version__,
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
    extra_requires = {
        "dev": [
            "rich",
            "pytest",
            "pytest-cov",
        ]
    }
)