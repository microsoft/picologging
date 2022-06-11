# picologging

**INTERNAL NOTE** This project is about 40% complete, do not deploy yet. 

Picologging is a high-performance logging library for Python. picologging is 10-20x faster than the `logging` module in the standard library.

Picologging is designed to be used as a _drop-in_ replacement for applications which already use logging, and supports the same API as the `logging` module.

## Installation

```console
$ pip install picologging
```

## Usage

Run `picologging.install()` once your logging setup is configured, (typically after calling `logging.basicSetup()`).

This patches all the loggers registered to use picologging loggers and formatters.

```python
import logging
logging.basicSetup()

import picologging; picologging.install()  # Add this line

logger = logging.getLogger()

logger.info("A log message!")

logger.warning("A log message with %s", "arguments")
```

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
