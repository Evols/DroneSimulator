import time
from contextlib import contextmanager

# Static start time shared across all calls
_start_time = time.time()


def elapsed():
    """Get elapsed time in seconds since start."""
    return time.time() - _start_time


def elapsed_str():
    # Add milliseconds to the format, round to 3 decimal places
    milliseconds = int((elapsed() % 1) * 1000)
    return time.strftime("%H:%M:%S", time.gmtime(elapsed())) + f".{milliseconds:03d}"


@contextmanager
def timer(description: str = "Operation"):
    """Context manager to time code execution."""
    start_time = time.time()
    try:
        yield
    finally:
        elapsed_time = time.time() - start_time
        # Format as hours:minutes:seconds.subseconds
        hours = int(elapsed_time // 3600)
        minutes = int((elapsed_time % 3600) // 60)
        seconds = int(elapsed_time % 60)
        subseconds = int((elapsed_time % 1) * 100)
        print(f"{description} took {hours:02d}:{minutes:02d}:{seconds:02d}.{subseconds:02d}")
