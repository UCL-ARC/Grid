import pytest

def pytest_addoption(parser):
    parser.addoption(
        "--test_name", action="store", help="File name of the test"
    )
    parser.addoption(
        "--grid", action="store", help="Grid configuration"
    )
    parser.addoption(
        "--mpi", action="store", help="MPI configuration"
    )

@pytest.fixture
def test_name(request):
    return request.config.getoption("--test_name")

@pytest.fixture
def grid(request):
    return request.config.getoption("--grid")

@pytest.fixture
def mpi(request):
    return request.config.getoption("--mpi")

@pytest.fixture
def cleanup_files():
    import os

    def _cleanup(failed=True):  
        if not failed: 
            if os.path.exists("output.txt"): os.remove("output.txt")
        if os.path.exists("ckpoint_rng.1"): os.remove("ckpoint_rng.1")
        if os.path.exists("ckpoint_lat.1"): os.remove("ckpoint_lat.1")

    return _cleanup