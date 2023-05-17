import picologging


def test():
    perc = picologging.PercentStyle("%(msg)s %(levelno)d %(name)s")
    record = picologging.LogRecord(
        "test", picologging.INFO, __file__, 1, "hello", (), None, None, None
    )
    assert perc.format(record) == "hello 20 test"

    try:
        perc.format(None)
    except:
        pass
    try:
        perc.format("")
    except:
        pass

    try:
        perc.format({})
    except:
        pass


if __name__ == "__main__":
    for _ in range(1000):
        test()
