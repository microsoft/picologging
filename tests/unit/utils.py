def filter_gc(stack):
    for frame in stack.frames[:4]:
        if "picologging" in frame.filename:
            return True

    return False
