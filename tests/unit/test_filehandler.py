import picologging

def test_filehandler(tmp_path):
    log_file = tmp_path / 'log.txt'
    handler = picologging.FileHandler(log_file)
    logger = picologging.getLogger('test')
    logger.setLevel(picologging.DEBUG)
    logger.addHandler(handler)
    logger.warning('test')
    handler.close()

    with open(log_file, 'r') as f:
        assert f.read() == "test\n"
