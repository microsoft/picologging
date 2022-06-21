# picologging

**Status** This project is an early-alpha. There are some incomplete features (see Issues).

Picologging is a high-performance logging library for Python. picologging is 4-10x faster than the `logging` module in the standard library.

Picologging is designed to be used as a _drop-in_ replacement for applications which already use logging, and supports the same API as the `logging` module.

## Installation

```console
$ pip install picologging
```

## Usage

Import `picologging as logging` to use picologging instead of the standard library logging module.

This patches all the loggers registered to use picologging loggers and formatters.

```python
import picologging as logging
logging.basicSetup()

logger = logging.getLogger()

logger.info("A log message!")

logger.warning("A log message with %s", "arguments")
```

## Benchmarks

Run `richbench benchmarks/` with the richbench CLI to see the benchmarks, here is a sample on macOS 11:

|                             Benchmark | Min     | Max     | Mean    | Min (+)         | Max (+)         | Mean (+)        |
|---------------------------------------|---------|---------|---------|-----------------|-----------------|-----------------|
|                           LogRecord() | 0.228   | 0.244   | 0.234   | 0.031 (7.5x)    | 0.031 (7.8x)    | 0.031 (7.6x)    |
|                  Formatter().format() | 0.077   | 0.079   | 0.078   | 0.005 (15.3x)   | 0.005 (14.9x)   | 0.005 (15.1x)   |
|        Formatter().format() with date | 0.299   | 0.359   | 0.313   | 0.083 (3.6x)    | 0.092 (3.9x)    | 0.086 (3.6x)    |
|           Logger(level=DEBUG).debug() | 0.725   | 0.741   | 0.730   | 0.069 (10.6x)   | 0.070 (10.6x)   | 0.069 (10.6x)   |
| Logger(level=DEBUG).debug() with args | 0.750   | 0.757   | 0.754   | 0.090 (8.3x)    | 0.095 (8.0x)    | 0.093 (8.1x)    |
|            Logger(level=INFO).debug() | 0.014   | 0.015   | 0.015   | 0.003 (4.1x)    | 0.004 (3.7x)    | 0.004 (3.9x)    |
|  Logger(level=INFO).debug() with args | 0.016   | 0.017   | 0.016   | 0.004 (4.1x)    | 0.004 (4.2x)    | 0.004 (4.1x)    |

## Limitations

See [docs/limitations.md](docs/limitations.md)

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

This project may contain trademarks or logos for projects, products, or services. Authorized use of Microsoft 
trademarks or logos is subject to and must follow 
[Microsoft's Trademark & Brand Guidelines](https://www.microsoft.com/en-us/legal/intellectualproperty/trademarks/usage/general).
Use of Microsoft trademarks or logos in modified versions of this project must not cause confusion or imply Microsoft sponsorship.
Any use of third-party trademarks or logos are subject to those third-party's policies.
