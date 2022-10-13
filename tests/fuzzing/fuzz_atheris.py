import io

import atheris

with atheris.instrument_imports():
    import sys

    import picologging

    picologging.basicConfig()
    logger = picologging.Logger("fuzz", picologging.DEBUG)
    tmp = io.StringIO()
    logger.addHandler(picologging.StreamHandler(tmp))


def TestOneInput(data):
    fdp = atheris.FuzzedDataProvider(data)
    logger.warning(fdp.ConsumeUnicode(1))


atheris.Setup(sys.argv, TestOneInput)
atheris.Fuzz()
