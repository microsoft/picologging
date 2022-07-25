import os
import pickle
import socket
import struct
import tempfile
import threading
from socketserver import ThreadingTCPServer, StreamRequestHandler

import pytest

import picologging
from picologging.handlers import SocketHandler


class ControlMixin:
    def __init__(self, handler, poll_interval):
        self._thread = None
        self._handler = handler
        self.poll_interval = poll_interval
        self.ready = threading.Event()

    def start(self):
        self._thread = threading.Thread(
            target=self.serve_forever, args=(self.poll_interval,)
        )
        self._thread.daemon = True
        self._thread.start()

    def serve_forever(self, poll_interval):
        self.ready.set()
        super().serve_forever(poll_interval)

    def stop(self):
        self.shutdown()
        if self._thread is not None:
            self._thread.join()
            self._thread = None
        self.server_close()
        self.ready.clear()


class TCPServer(ControlMixin, ThreadingTCPServer):
    allow_reuse_address = True

    def __init__(self, addr, poll_interval=0.5, bind_and_activate=True):
        class DelegatingTCPRequestHandler(StreamRequestHandler):
            def handle(self):
                self.server._handler(self)

        ThreadingTCPServer.__init__(
            self, addr, DelegatingTCPRequestHandler, bind_and_activate
        )
        ControlMixin.__init__(self, self.handle_socket, poll_interval)
        self.log_output = ""
        self.handled = threading.Semaphore(0)

    def server_bind(self):
        super().server_bind()
        self.port = self.socket.getsockname()[1]

    def handle_socket(self, request):
        conn = request.connection
        while True:
            chunk = conn.recv(4)
            if len(chunk) < 4:
                break
            slen = struct.unpack(">L", chunk)[0]
            chunk = conn.recv(slen)
            while len(chunk) < slen:
                chunk = chunk + conn.recv(slen - len(chunk))
            obj = pickle.loads(chunk)
            record = picologging.makeLogRecord(obj)
            self.log_output += record.msg + "\n"
            self.handled.release()


if hasattr(socket, "AF_UNIX"):

    class UnixStreamServer(TCPServer):
        address_family = socket.AF_UNIX


def test_sockethandler():
    server = TCPServer(("localhost", 0), 0.01)
    server.start()
    server.ready.wait()

    handler = SocketHandler("localhost", server.port)
    logger = picologging.getLogger("test")
    logger.setLevel(picologging.DEBUG)
    logger.addHandler(handler)

    logger.error("test")
    server.handled.acquire()
    logger.debug("test")
    server.handled.acquire()

    assert server.log_output == "test\ntest\n"

    handler.close()
    server.stop()


@pytest.mark.skipif(not hasattr(socket, "AF_UNIX"), reason="Unix sockets required")
def test_unix_sockethandler():
    address = tempfile.NamedTemporaryFile(prefix="picologging_", suffix=".sock").name
    server = UnixStreamServer(address, 0.01)
    server.start()
    server.ready.wait()

    handler = SocketHandler(address, None)
    logger = picologging.getLogger("test")
    logger.setLevel(picologging.DEBUG)
    logger.addHandler(handler)

    logger.error("test")
    server.handled.acquire()
    logger.debug("test")
    server.handled.acquire()

    assert server.log_output == "test\ntest\n"

    handler.close()
    server.stop()
    os.remove(address)


@pytest.mark.skipif(not hasattr(socket, "AF_UNIX"), reason="Unix sockets required")
def test_unix_sockethandler_connect_exception(monkeypatch):
    def mock_os_error(*args):
        raise OSError()

    monkeypatch.setattr(socket.socket, "connect", mock_os_error)

    address = tempfile.NamedTemporaryFile(prefix="picologging_", suffix=".sock").name
    handler = SocketHandler(address, None)
    handler.retryMax = 1
    logger = picologging.getLogger("test")
    logger.setLevel(picologging.DEBUG)
    logger.addHandler(handler)

    logger.debug("test")

    handler.retryTime -= 5
    logger.debug("test")

    assert handler.retryTime is not None
    assert handler.sock is None

    handler.close()


@pytest.mark.skipif(not hasattr(socket, "AF_UNIX"), reason="Unix sockets required")
def test_unix_sockethandler_emit_exception(monkeypatch):
    def mock_exception(*args):
        raise Exception()

    monkeypatch.setattr(socket.socket, "sendall", mock_exception)

    address = tempfile.NamedTemporaryFile(prefix="picologging_", suffix=".sock").name
    server = UnixStreamServer(address, 0.01)
    server.start()
    server.ready.wait()

    handler = SocketHandler(address, None)
    logger = picologging.getLogger("test")
    logger.setLevel(picologging.DEBUG)
    logger.addHandler(handler)

    logger.debug("test")

    try:
        1 / 0
    except ZeroDivisionError:
        logger.exception("error")

    handler.closeOnError = True
    logger.debug("test")
    assert handler.sock is None

    handler.close()
    server.stop()
    os.remove(address)
