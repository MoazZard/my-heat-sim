#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Declaration of get_temperature from heat.c
double get_temperature(int N, int maxIter, double radTemp, double tolerance);

// Declarations of file reading/writing functions from file_reader.c
int read_dims(char *filename);
double *read_array(char *filename, int numOfValues);
void *write_to_output_file(char *filename, double *output, int numOfValues);

static void usage(const char *prog)
{
    fprintf(stderr, "Usage: %s <N> <maxiter> <input file> <output file>\n", prog);
}

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    int N = atoi(argv[1]);             // grid size
    int maxIter = atoi(argv[2]);       // maximum iterations
    const char *input_file = argv[3];  // file containing radiator temperatures (one per line)
    const char *output_file = argv[4]; // file to write results to

    if (N <= 0 || maxIter <= 0) // basic validation of command line arguments
    {
        fprintf(stderr, "N and maxiter must be positive integers\n");
        return EXIT_FAILURE;
    }

    // Use reader.c to read input and write output.
    // First line of the input file contains the number of temperatures.
    int count = read_dims((char *)input_file);
    if (count <= 0)
    {
        fprintf(stderr, "Invalid number of values in input file %s\n", input_file);
        return EXIT_FAILURE;
    }

    double *rad_temps = read_array((char *)input_file, count);
    if (!rad_temps)
    {
        fprintf(stderr, "Failed to read radiator temperatures from %s\n", input_file);
        return EXIT_FAILURE;
    }

    // Prepare output array
    double *results = malloc(count * sizeof(double));
    if (!results)
    {
        perror("malloc results");
        free(rad_temps);
        return EXIT_FAILURE;
    }

    const double tolerance = 1e-9;

    // For each radiator temperature call get_temperature sequentially (distribute using MPI)
    for (int i = 0; i < count; i++)
    {
        double rad = rad_temps[i];
        double res = get_temperature(N, maxIter, rad, tolerance);
        results[i] = res;
    }

    // Write results to output file using provided writer (writes same format as input)
    write_to_output_file((char *)output_file, results, count);

    free(rad_temps);
    free(results);

    return EXIT_SUCCESS;
}

// for makefile
// gcc -std=c11 -O2 heat.c main_nearly.c -o program && ./program 10 100 test_input.txt test_output.txt
