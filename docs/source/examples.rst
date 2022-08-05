.. _examples:

Examples
========

Basic logging
-------------

The most basic usage for picologging is to call the debug, info, warning, error, critical and exception functions directly on the picologging module:

.. code-block:: python

    import picologging

    picologging.basicConfig(level=picologging.DEBUG)
    picologging.debug("This is a debug message")
    picologging.info("This is an info message")
    picologging.warning("This is a warning message")
    picologging.error("This is an error message")
    picologging.critical("This is a critical message")

This will use the default handler and formatter. You can specify a different formatter with the formatter keyword argument:

.. code-block:: python

    import picologging

    picologging.basicConfig(level=picologging.DEBUG, formatter=picologging.Formatter("%(levelname)s:%(message)s"))
    picologging.debug("This is a debug message")

    # Output:
    # DEBUG:This is a debug message

Using custom handlers
---------------------

Picologging has custom handlers beyond the StreamHandler and FileHandler. You can write your own handler by implementing the Handler class.

There are a collection of pre-built handlers in the :ref:`handlers` module.
