#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Declaration of get_temperature from heat.c
double get_temperature(int N, int maxIter, double radTemp, double tolerance);

// Declarations of file reading/writing functions from file_reader.c
int read_dims(char *filename);
double *read_array(char *filename, int numOfValues);
void *write_to_output_file(char *filename, double *output, int numOfValues);

static void usage(const char *prog) // gets printed if the user provides incorrect command line arguments
{
    fprintf(stderr, "Usage: %s <N> <maxiter> <input file> <output file>\n", prog);
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv); // Initialize MPI environment

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Get the rank of the process
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Get the total number of processes

    int N, maxIter, count; // N and maxIter are the same for all ranks, count is the number of radiator temperatures (also the same for all ranks)
    double *rad_temps = NULL; // array to hold input radiator temperatures
    double *results = NULL; // array for results
    const double tolerance = 1e-9;

    // rank 0 reads and allocated the data, then broadcasts to all other ranks
    if (rank == 0) {
        if (argc != 5) {
            usage(argv[0]);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        N = atoi(argv[1]);
        maxIter = atoi(argv[2]);

        const char *input_file = argv[3];
        const char *output_file = argv[4];

        count = read_dims((char *)input_file);
        rad_temps = read_array((char *)input_file, count);

        results = malloc(count * sizeof(double));
    } 

    // broadcast the values of N, maxIter, count, and the array of radiator temperatures to all ranks
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&maxIter, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&count, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // allocate memory for input array on non-root ranks and broadcast it 
    if (rank != 0) {
        rad_temps = malloc(count * sizeof(double));
    }
    MPI_Bcast(rad_temps, count, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double *local_results = malloc(count * sizeof(double)); // local results for each rank for reduction

    // initialise to 0 (important for reduction)
    for (int i = 0; i < count; i++) {
        local_results[i] = 0.0;
    }

    // each rank computes a portion of the results based on its rank and the total number of ranks (size)
    for (int i = rank; i < count; i += size) {
        local_results[i] = get_temperature(N, maxIter, rad_temps[i], tolerance);
    }

    // reduce local results from ranks into results array on rank 0
    MPI_Reduce(local_results, results, count, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        write_to_output_file((char *)argv[4], results, count);
    }

    free(rad_temps);
    free(local_results);

    if (rank == 0) {
        free(results);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}

