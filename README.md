# picologging

[![PyPI - Python Version](https://img.shields.io/pypi/pyversions/picologging)](https://pypi.org/project/picologging/) 
[![PyPI](https://img.shields.io/pypi/v/picologging)](https://pypi.org/project/picologging/)
[![codecov](https://codecov.io/gh/microsoft/picologging/branch/main/graph/badge.svg?token=KHs6FpQlVW)](https://codecov.io/gh/microsoft/picologging)

**Status** This project is an *early-alpha*. There are some incomplete features (see Issues).

Picologging is a high-performance logging library for Python. picologging is 4-10x faster than the `logging` module in the standard library.

Picologging is designed to be used as a *drop-in* replacement for applications which already use logging, and supports the same API as the `logging` module.

Check out the [Documentation](https://microsoft.github.io/picologging/) for more.

## Installation

```console
$ pip install picologging
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
|                         FileHandler() | 0.138   | 0.151   | 0.143   | 0.055 (2.5x)    | 0.063 (2.4x)    | 0.058 (2.5x)    |
|                  WatchedFileHandler() | 0.189   | 0.197   | 0.193   | 0.097 (1.9x)    | 0.101 (1.9x)    | 0.099 (1.9x)    |
|                 RotatingFileHandler() | 0.287   | 0.304   | 0.296   | 0.174 (1.6x)    | 0.178 (1.7x)    | 0.176 (1.7x)    |
|                        QueueHandler() | 1.109   | 1.195   | 1.130   | 0.142 (7.8x)    | 0.151 (7.9x)    | 0.147 (7.7x)    |
|      QueueListener() + QueueHandler() | 0.157   | 0.167   | 0.162   | 0.034 (4.6x)    | 0.039 (4.3x)    | 0.037 (4.3x)    |
|                       MemoryHandler() | 0.126   | 0.144   | 0.133   | 0.051 (2.5x)    | 0.059 (2.5x)    | 0.054 (2.5x)    |
|                           LogRecord() | 0.225   | 0.248   | 0.233   | 0.026 (8.7x)    | 0.029 (8.5x)    | 0.028 (8.4x)    |
|                  Formatter().format() | 0.076   | 0.086   | 0.081   | 0.004 (18.7x)   | 0.005 (18.9x)   | 0.004 (19.1x)   |
|        Formatter().format() with date | 0.298   | 0.311   | 0.304   | 0.081 (3.7x)    | 0.087 (3.6x)    | 0.084 (3.6x)    |
|           Logger(level=DEBUG).debug() | 0.726   | 0.743   | 0.734   | 0.059 (12.3x)   | 0.061 (12.3x)   | 0.060 (12.3x)   |
| Logger(level=DEBUG).debug() with args | 0.761   | 0.809   | 0.777   | 0.081 (9.4x)    | 0.087 (9.3x)    | 0.084 (9.2x)    |
|            Logger(level=INFO).debug() | 0.016   | 0.018   | 0.017   | 0.004 (4.3x)    | 0.005 (3.8x)    | 0.004 (4.1x)    |
|  Logger(level=INFO).debug() with args | 0.018   | 0.019   | 0.018   | 0.005 (3.8x)    | 0.005 (3.8x)    | 0.005 (3.7x)    |

## Limitations

See [Limitations](https://microsoft.github.io/picologging/limitations.html)

## Contributing

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.opensource.microsoft.com.

When you submit a pull request, a CLA bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., status check, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

## Trademarks

Some components of this Python package are from CPython 3.11 logging library for compatibility reasons.

CPython 3.11 is licensed under the PSF license.
The logging module is Copyright (C) 2001-2019 Vinay Sajip. All Rights Reserved.

This project may contain trademarks or logos for projects, products, or services. Authorized use of Microsoft 
trademarks or logos is subject to and must follow 
[Microsoft's Trademark & Brand Guidelines](https://www.microsoft.com/en-us/legal/intellectualproperty/trademarks/usage/general).
Use of Microsoft trademarks or logos in modified versions of this project must not cause confusion or imply Microsoft sponsorship.
Any use of third-party trademarks or logos are subject to those third-party's policies.
