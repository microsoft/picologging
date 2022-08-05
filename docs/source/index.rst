.. picologging documentation master file, created by
   sphinx-quickstart on Fri Aug  5 15:34:22 2022.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

picologging
===========

**Status** This project is an *early-alpha*. There are some incomplete features (see Issues).

Picologging is a high-performance logging library for Python. picologging is 4-10x faster than the `logging` module in the standard library.

Picologging is designed to be used as a *drop-in* replacement for applications which already use logging, and supports the same API as the `logging` module.

Installation
------------

.. code-block:: console

   $ pip install picologging

Usage
-----

Import `picologging as logging` to use picologging instead of the standard library logging module.

This patches all the loggers registered to use picologging loggers and formatters.

.. code-block:: python

   import picologging as logging
   logging.basicConfig()

   logger = logging.getLogger()

   logger.info("A log message!")

   logger.warning("A log message with %s", "arguments")


Limitations
-----------

See :ref:`limitations`.

Contributing
------------

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.opensource.microsoft.com.

When you submit a pull request, a CLA bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., status check, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the `Microsoft Open Source Code of Conduct <https://opensource.microsoft.com/codeofconduct/>`_.
For more information see the `Code of Conduct FAQ <https://opensource.microsoft.com/codeofconduct/faq/>`_ or
contact `opencode@microsoft.com <mailto:opencode@microsoft.com>`_ with any additional questions or comments.

Trademarks
----------

Some components of this Python package are from CPython 3.11 logging library for compatibility reasons.

CPython 3.11 is licensed under the PSF license.
The logging module is Copyright (C) 2001-2019 Vinay Sajip. All Rights Reserved.

This project may contain trademarks or logos for projects, products, or services. Authorized use of Microsoft 
trademarks or logos is subject to and must follow 
`Microsoft's Trademark & Brand Guidelines <https://www.microsoft.com/en-us/legal/intellectualproperty/trademarks/usage/general>`_.
Use of Microsoft trademarks or logos in modified versions of this project must not cause confusion or imply Microsoft sponsorship.
Any use of third-party trademarks or logos are subject to those third-party's policies.



.. toctree::
   :glob:
   :maxdepth: 3
   :hidden:

   logging
   handlers
   examples
   limitations
