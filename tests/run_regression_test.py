import pytest

def read_expected(test_name="Test_hmc_Sp_WilsonFundFermionGauge", grid="8.8.8.8", mpi="1.1.1.1"):
    """
    Read expected values from file.

    The file contains one or more entries of the following format:
    <grid> <mpi> <plaquette> <checksum_rng> <checksum_lat>
    Eg.
    8.8.8.8 1.1.1.1 0.0256253844 922c392f d1e4cc1c
    """

    with open(f"{test_name}_expected.txt") as file:
        for line in file:
            line_split = line.split()
            if line_split[0] == grid and line_split[1] == mpi:
                return float(line_split[2]), line_split[3], line_split[4]


def read_output():
    """
    Read test output and fish out values of interest.
    """

    checksum_rng = None
    checksum_lat = None
    plaquette = None
    with open("output.txt", 'r') as file:
        for line in file:
            if "Written NERSC" in line:
                subline = line.split('checksum ')[1]
                if len(subline.split()) == 1: # this is the rng checksum line
                    checksum_rng = subline.strip()
                elif len(subline.split()) == 3: # this is the lat checksum and plaquette value line
                    checksum_lat = subline.split()[0]
                    plaquette = float(subline.split()[2])
                else:
                    print("Picked wrong line...")

    if (checksum_rng is None) or (checksum_lat is None) or (plaquette is None):
        print("Error reading values from output file. Make sure you compile the test with CPparams.saveInterval=1 in order to produce the required output.")
        exit(1)
        
    return plaquette, checksum_rng, checksum_lat


@pytest.fixture
def cl_arguments():
    import argparse

    parser = argparse.ArgumentParser(description='Run end-to-end tests and compare results with expectations.')
    parser.add_argument("test_name", help="File name of the test")
    parser.add_argument("grid", help="Grid configuration")
    parser.add_argument("mpi", help="MPI configuration")
    parser.add_argument("-s", "--stop", action='store_true', help="Flag to stop testing when a test fails.")
    args = parser.parse_args()

    return args


#def test_outputs(cl_arguments):
def test_outputs(test_name, grid, mpi):
    import subprocess
    import os

    #test_name = cl_arguments.test_name
    #expected_plaquette, expected_checksum_rng, expected_checksum_lat = read_expected(test_name, cl_arguments.grid, cl_arguments.mpi)
    expected_plaquette, expected_checksum_rng, expected_checksum_lat = read_expected(test_name, grid, mpi)

    #result = subprocess.run([f"./{test_name} --grid {cl_arguments.grid} --mpi {cl_arguments.mpi} --Thermalizations 0 --Trajectories 1 --threads 1 > output.txt"], shell=True, encoding="text")
    #result = subprocess.run([f"./{test_name} --grid {grid} --mpi {mpi} --Thermalizations 0 --Trajectories 1 --threads 1 > output.txt"], shell=True, encoding="text")
    result = subprocess.run([f"./{test_name} --grid {grid} --mpi {mpi} --Thermalizations 0 --Trajectories 1 > output.txt"], shell=True, encoding="text")
    plaquette, checksum_rng, checksum_lat = read_output()

    print(f"Running {test_name}")
    assert plaquette == expected_plaquette
    assert checksum_rng == expected_checksum_rng
    assert checksum_lat == expected_checksum_lat
    #result = compare(plaquette, expected_plaquette, "plaquette", args.stop)
    #result = result and compare(checksum_rng, expected_checksum_rng, "Checksum RNG file ", args.stop)
    #result = result and compare(checksum_lat, expected_checksum_lat, "Checksum LAT file ", args.stop)
    # if result:
    #     print("All tests passed!")
    #     os.remove("output.txt")
    # else:
    #     print("Some tests failed...")

    # os.remove("ckpoint_rng.1")
    # os.remove("ckpoint_lat.1")
