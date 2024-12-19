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

@pytest.fixture(scope="function", autouse=True)
def cleanup_files():
    import os

    print("Before!")
    yield
    print(f"After!")
    os.remove("output.txt")

    os.remove("ckpoint_rng.1")
    os.remove("ckpoint_lat.1")