import pytest
import pytest_check as check

def read_expected_values_line(test_name, line_number=1):
    """
    Read test parameters and expected values from file.

    The file contains one or more entries of the following format:
    <line_number> <grid> <mpi> <nthreads> <MDsteps> <trajL> <CPU|GPU> <plaquette> <checksum_rng> <checksum_lat>
    Eg.
    1 8.8.8.8 1.1.1.1 1 2 0.1 CPU 0.0256253844 922c392f d1e4f305

    This function will return the test parameters and expected values 
    only for the requested line_number.
    """

    test_parameters = {}
    with open(f"{test_name}_expected.txt") as file:
        for line in file:
            line_split = line.split()
            if line_split[0] == line_number:
                test_parameters['grid'] = line_split[1]
                test_parameters['mpi'] = line_split[2]
                test_parameters['nthreads'] = line_split[3]
                test_parameters['MDsteps'] = line_split[4]
                test_parameters['trajL'] = line_split[5]
                test_parameters['CPU|GPU'] = line_split[6]
                print("Reading reference values for ", test_parameters)
                return test_parameters, float(line_split[7]), line_split[8], line_split[9]

    return test_parameters, None, None, None


def read_output(test_parameters):
    """
    Read test output and fish out values of interest.
    """

    MDsteps = None
    trajL = None
    CPUvsGPU = 'CPU'
    checked_CPUvsGPU = False
    checksum_rng = None
    checksum_lat = None
    plaquette = None

    with open("output.txt", 'r') as file:
        for line in file:
        # Check that the test was run with the expected parameters
            if not checked_CPUvsGPU:
                if "cuda" in line:
                    CPUvsGPU = 'GPU'
                    checked_CPUvsGPU = True
            if "Number of MD steps" in line:
                MDsteps = line.split(' : ')[4].strip()
                if MDsteps != test_parameters['MDsteps']:
                    pytest.fail(f"Test was run with MDsteps={MDsteps} instead of {test_parameters['MDsteps']}")
            elif "Trajectory length" in line:
                trajL = line.split(' : ')[4].strip()
                if trajL != test_parameters['trajL']:
                    pytest.fail(f"Test was run with trajL={trajL} instead of {test_parameters['trajL']}")
        # Read the values to test
            elif "Written NERSC" in line:
                subline = line.split('checksum ')[1]
                if len(subline.split()) == 1: # this is the rng checksum line
                    checksum_rng = subline.strip()
                elif len(subline.split()) == 3: # this is the lat checksum and plaquette value line
                    checksum_lat = subline.split()[0]
                    plaquette = float(subline.split()[2])
                else:
                    print("Picked wrong line...")

    if CPUvsGPU != test_parameters['CPU|GPU']:
        pytest.fail(f"Test was run with {CPUvsGPU} instead of {test_parameters['CPU|GPU']}")
    if (MDsteps is None) or (trajL is None):
        pytest.fail("Could not verify test parameters MDsteps and/or trajL against test output.")

    return plaquette, checksum_rng, checksum_lat


def test_outputs(test_name, expected_line, cleanup_files):
    import subprocess
    import os

    test_parameters, expected_plaquette, expected_checksum_rng, expected_checksum_lat = read_expected_values_line(test_name, expected_line)
    if (not test_parameters) or (expected_plaquette is None) or (expected_checksum_rng is None) or (expected_checksum_lat is None):
        pytest.fail(f"No appropriate reference values found, check {test_name}_expected.txt")

    print(f"Running {test_name} for test parameters: ", test_parameters)
    if test_parameters['nthreads'] == '0':
        result = subprocess.run([f"./{test_name} --grid {test_parameters['grid']} --mpi {test_parameters['mpi']} --Thermalizations 0 --Trajectories 1 > output.txt"], shell=True, encoding="text")
    else:
        result = subprocess.run([f"./{test_name} --grid {test_parameters['grid']} --mpi {test_parameters['mpi']} --Thermalizations 0 --Trajectories 1 --threads {test_parameters['nthreads']} > output.txt"], shell=True, encoding="text")
    plaquette, checksum_rng, checksum_lat = read_output(test_parameters)
    if (checksum_rng is None) or (checksum_lat is None) or (plaquette is None):
        pytest.fail("Error reading values from output file. Make sure you compile the test with CPparams.saveInterval=1 in order to produce the required output.")
        
    # This manual check of each condition doesn't have to happen for pytest-check
    # version 1.2.0 and later. We can use any_failures() instead.
    failed = False
    if not check.equal(plaquette, expected_plaquette, msg="Plaquette value comparison failed") : failed = True
    if not check.equal(checksum_lat, expected_checksum_lat, msg="LAT file checksum comparison failed") : failed = True
    if not check.equal(checksum_rng, expected_checksum_rng, msg="RND file checksum comparison failed") : failed = True

    cleanup_files(failed)

