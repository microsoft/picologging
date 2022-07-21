import atheris
import io

with atheris.instrument_imports():
    import picologging
    import sys

    picologging.basicConfig()
    logger = picologging.Logger("fuzz", picologging.DEBUG)
    tmp = io.StringIO()
    logger.addHandler(picologging.StreamHandler(tmp))


def TestOneInput(data):
    fdp = atheris.FuzzedDataProvider(data)
    logger.warning(fdp.ConsumeUnicode(1))


atheris.Setup(sys.argv, TestOneInput)
atheris.Fuzz()
