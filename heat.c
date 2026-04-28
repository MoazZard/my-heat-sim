#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#ifdef _OPENMP
#include <omp.h>
#endif

// debugging function to print the room temperatures
void print_room(int N, double t[N][N])
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            printf("%.2f ", t[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

// Function to compute the temperature at the center of the room after simulating heat distribution
double get_temperature(int N, int maxIter, double radTemp, double tolerance)
{
    double (*t)[N] = malloc(sizeof(double[N][N]));

    int start = (int)(0.2 * N);
    int end = (int)(0.8 * N);

    // Initialize
    #pragma omp parallel for collapse(2) schedule(static) // parallelize so its not the master thread touching all the data, collapse(2) allows us to parallelize both loops together
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            if (j == N - 1 && i >= start && i < end) // if on the right wall and within the heated region
                t[i][j] = radTemp;
            else
                t[i][j] = 10.0; // initial temperature for other points
        }
    }

    // Iterations
    double maxDiff;
    for (int iter = 0; iter < maxIter; iter++)
    {
        maxDiff = 0.0;

        // WHITE pass
        #pragma omp parallel for schedule(static) reduction(max : maxDiff)
        for (int i = 1; i < N - 1; i++)
        {
            for (int j = 1 + (i % 2); j < N - 1; j += 2)
            {
                double new_val = 0.25 * (t[i - 1][j] + t[i + 1][j] +
                                         t[i][j - 1] + t[i][j + 1]); // new value is the average of the four neighbors

                double diff = fabs(new_val - t[i][j]);
                if (diff > maxDiff)
                    maxDiff = diff;

                t[i][j] = new_val;
            }
        }

        // BLACK pass
        #pragma omp parallel for schedule(static) reduction(max : maxDiff)
        for (int i = 1; i < N - 1; i++)
        {
            for (int j = 1 + ((i + 1) % 2); j < N - 1; j += 2)
            {
                double new_val = 0.25 * (t[i - 1][j] + t[i + 1][j] +
                                         t[i][j - 1] + t[i][j + 1]);

                double diff = fabs(new_val - t[i][j]);
                if (diff > maxDiff)
                    maxDiff = diff;

                t[i][j] = new_val;
            }
        }

        // Check for convergence
        if (maxDiff < tolerance)
        {
            printf("Converged after %d iterations\n", iter);
            break;
        }
    }

    int pointx = (int)floor((N - 1) * 0.5);
    int pointy = (int)floor((N - 1) * 0.5);
    double result = t[pointx][pointy];

    free(t); // free allocated memory

    return result;
}
