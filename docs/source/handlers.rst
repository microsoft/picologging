.. _handlers:

Handlers
========

Base Handler
------------

.. autoclass:: picologging.Handler
   :members:
   :member-order: bysource

Watched File Handler
--------------------

.. autoclass:: picologging.handlers.WatchedFileHandler
   :members:
   :member-order: bysource

Base Rotating Handler
---------------------

.. autoclass:: picologging.handlers.BaseRotatingHandler
   :members:
   :member-order: bysource

Rotating File Handler
---------------------

.. autoclass:: picologging.handlers.RotatingFileHandler
   :members:
   :member-order: bysource

Timed Rotating File Handler
---------------------------

.. autoclass:: picologging.handlers.TimedRotatingFileHandler
   :members:
   :member-order: bysource

Queue Handler
-------------

.. autoclass:: picologging.handlers.QueueHandler
   :members:
   :member-order: bysource

Queue Listener
--------------

The queue listener and queue handler can be combined for non-blocking logging, for example:

.. code-block:: python

    logger = picologging.Logger("test", picologging.DEBUG)
    stream = io.StringIO()
    stream_handler = picologging.StreamHandler(stream)
    q = queue.Queue()
    listener = QueueListener(q, stream_handler)
    listener.start()
    handler = QueueHandler(q)
    logger.addHandler(handler)
    logger.debug("test")

    listener.stop()
    assert stream.getvalue() == "test\n"


.. autoclass:: picologging.handlers.QueueListener
   :members:
   :member-order: bysource

Buffering Handler
-----------------

.. autoclass:: picologging.handlers.BufferingHandler
   :members:
   :member-order: bysource

Memory Handler
--------------

.. autoclass:: picologging.handlers.MemoryHandler
   :members:
   :member-order: bysource

Socket Handler
--------------

.. autoclass:: picologging.handlers.SocketHandler
   :members:
   :member-order: bysource
