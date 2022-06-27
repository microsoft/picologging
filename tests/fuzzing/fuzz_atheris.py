import atheris

from picologging import StreamHandler
import io

with atheris.instrument_imports():
  import picologging
  import sys
  picologging.basicConfig()
  logger = picologging.Logger("fuzz", picologging.DEBUG)
  tmp = io.StringIO()
  logger.addHandler(StreamHandler(tmp))


def TestOneInput(data):
    fdp = atheris.FuzzedDataProvider(data)
    logger.warning(fdp.ConsumeUnicode())
    logger.warning(data)

atheris.Setup(sys.argv, TestOneInput)
atheris.Fuzz()