# picologging

[![PyPI - Python Version](https://img.shields.io/pypi/pyversions/picologging)](https://pypi.org/project/picologging/)
[![PyPI](https://img.shields.io/pypi/v/picologging)](https://pypi.org/project/picologging/)
[![Anaconda-Server Badge](https://anaconda.org/conda-forge/picologging/badges/version.svg)](https://anaconda.org/conda-forge/picologging)
[![codecov](https://codecov.io/gh/microsoft/picologging/branch/main/graph/badge.svg?token=KHs6FpQlVW)](https://codecov.io/gh/microsoft/picologging)

> **Warning**
> This project is in *beta*.
> There are some incomplete features (see [Limitations](https://microsoft.github.io/picologging/limitations.html)).

Picologging is a high-performance logging library for Python. picologging is 4-17x faster than the `logging` module in the standard library.

Picologging is designed to be used as a *drop-in* replacement for applications which already use logging, and supports the same API as the `logging` module.

Check out the [Documentation](https://microsoft.github.io/picologging/) for more.

## Installation

Picologging can be installed from PyPi using pip:

```console
pip install picologging
```

Or from conda forge using conda:

```console
conda install -c conda-forge picologging
```

## Usage

Import `picologging as logging` to use picologging instead of the standard library logging module.

This patches all the loggers registered to use picologging loggers and formatters.

```python
import picologging as logging
logging.basicConfig()

logger = logging.getLogger()

logger.info("A log message!")

logger.warning("A log message with %s", "arguments")
```

## Benchmarks

Run `richbench benchmarks/ --markdown` with the richbench CLI to see the benchmarks, here is a sample on macOS 11:

|                             Benchmark | Min     | Max     | Mean    | Min (+)         | Max (+)         | Mean (+)        |
|---------------------------------------|---------|---------|---------|-----------------|-----------------|-----------------|
|           Logger(level=DEBUG).debug() | 0.569   | 0.600   | 0.578   | 0.031 (18.3x)   | 0.035 (17.0x)   | 0.033 (17.7x)   |
| Logger(level=DEBUG).debug() with args | 0.591   | 0.607   | 0.601   | 0.047 (12.5x)   | 0.050 (12.2x)   | 0.048 (12.4x)   |
|            Logger(level=INFO).debug() | 0.013   | 0.014   | 0.013   | 0.003 (5.0x)    | 0.003 (4.4x)    | 0.003 (4.8x)    |
|  Logger(level=INFO).debug() with args | 0.013   | 0.014   | 0.013   | 0.003 (4.6x)    | 0.003 (4.2x)    | 0.003 (4.4x)    |

## Limitations

See [Limitations](https://microsoft.github.io/picologging/limitations.html)

## Contributing

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit [cla.opensource.microsoft.com](https://cla.opensource.microsoft.com).

When you submit a pull request, a CLA bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., status check, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

## Local development

This project comes bundled with a dev container which sets up an appropriate environment. If you install the Dev Containers extension for VS Code, then opening this project in VS Code should prompt it to open it in the dev container.

Once opened in the dev container, run:

```console
pip install -e ".[dev]"
pre-commit install
python setup.py build_ext --inplace --build-type Debug
```

Run the build command whenever you make changes to the files.

It's also helpful to create a `.vscode/launch.json` file like this one:

```json
{
    "version": "0.2.0",
    "configurations": [
    {
        "name": "(gdb) Launch pytest",
        "type": "cppdbg",
        "request": "launch",
        "program": "/usr/local/bin/python",
        "args": ["-m", "pytest", "tests"],
        "stopAtEntry": false,
        "cwd": "${workspaceFolder}",
        "environment": [],
        "externalConsole": false,
        "MIMode": "gdb",
        "setupCommands": [
            {
                "description": "Enable pretty-printing for gdb",
                "text": "-enable-pretty-printing",
                "ignoreFailures": true
            },
            {
                "description":  "Set Disassembly Flavor to Intel",
                "text": "-gdb-set disassembly-flavor intel",
                "ignoreFailures": true
            },
        ]
    }
}
```

Now you can press the "Run and debug" button to run `pytest` from the `gdb` debugger
and use breakpoint debugging in the C code.

If you would like to be able to dive into the CPython code while debugging, then:

1. Do a git checkout of the tagged branch for the devcontainer's Python version
into the devcontainer's `/workspaces/` directory. You may need to `sudo`.
2. Follow the instructions in the CPython README to compile the code.
3. Add the following key to the the configuration in `launch.json`:

    ```json
    "sourceFileMap": { "/usr/src/python": "/workspaces/cpython" },
    ```

4. Add the following command to the `setupCommands` in `launch.json`:

    ```json
    {
        "description": "Find CPython source code",
        "text": "-gdb-set auto-load safe-path /workspaces/cpython"
    },
    ```

## Trademarks

Some components of this Python package are from CPython 3.11 logging library for compatibility reasons.

CPython 3.11 is licensed under the PSF license.
The logging module is Copyright (C) 2001-2019 Vinay Sajip. All Rights Reserved.

This project may contain trademarks or logos for projects, products, or services. Authorized use of Microsoft trademarks or logos is subject to and must follow [Microsoft's Trademark & Brand Guidelines](https://www.microsoft.com/en-us/legal/intellectualproperty/trademarks/usage/general).
Use of Microsoft trademarks or logos in modified versions of this project must not cause confusion or imply Microsoft sponsorship.
Any use of third-party trademarks or logos are subject to those third-party's policies.
