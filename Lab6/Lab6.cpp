/*
Author: Abby McCollam
Class: ECE4122 Section A
Last Date Modified: 11/30/23
Description: Using MPI to estimate value of definite integrals with Monte Carlo method
*/

//headers
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <mpi.h>
#include <unistd.h>

using namespace std;

double g1(double x) // function for first integral
{
    return x * x;
}

double g2(double x) //function for second integral
{
    return exp(-x * x);
}

//function for estimating the integral (pointer, lower bound, upper bound, number of samples, rank, size)
double calculate_integral(double (*g)(double), double a, double b, int num_samples, int rank, int size)
{
    srand(time(NULL) + rank); //random number generator

    int proc_num_samples = num_samples / size; //calculating number of samples for each processor

    // Each process generates local random samples
    double local_sum = 0.0;
    for (int i = 0; i < proc_num_samples; ++i) //looping through processor number of samples
    {
        double x = a + (b - a) * (rand() / (RAND_MAX + 1.0)); //generates random variable x on interval (a,b)
        local_sum += g(x); //evaluates g(x) at random x and accumulating values
    }

    double global_sum;
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD); //summing up over all processors and storing in root processor

    if (rank == 0) //only returning final integral estimate to root processor
    {
        return (b - a) * (global_sum / num_samples); //calculating final integral estimation
    }
    else
    {
        return 0.0;
    }
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv); //initializing MPI

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); //rank of communicator
    MPI_Comm_size(MPI_COMM_WORLD, &size); //size of communicator

    if (argc != 5) //error checking command line arguments
    {
        if (rank == 0)
        {
            cout << "Usage: " << argv[0] << " -P [1 or 2] -N [num_samples]" << endl;
        }
        MPI_Finalize(); //finalizing MPI
        return 1;
    }

    int option;
    int integral_option = 0;
    int num_samples = 0;

    while ((option = getopt(argc, argv, "P:N:")) != -1) //processing command line arguments
    {
        switch (option)
        {
            case 'P': //argument after P selects function option
                integral_option = atoi(optarg);
                break;
            case 'N': //argument after N represents number of samples
                num_samples = atoi(optarg);
                break;
            default:
                if (rank == 0)
                {
                    cout << "Usage: " << argv[0] << " -P [1 or 2] -N [num_samples]" << endl;
                }
                MPI_Finalize();
                return 1;
        }
    }

    double (*chose_function)(double);

    if (integral_option == 1) //selecting function 1
    {
        chose_function = g1;
    }
    else if (integral_option == 2) //selecting function 2
    {
        chose_function = g2;
    }
    else
    {
        if (rank == 0)
        {
            cout << "Invalid option for -P. Use 1 or 2." << endl;
        }
        MPI_Finalize(); //finalizing MPI
        return 1;
    }

    MPI_Bcast(&integral_option, 1, MPI_INT, 0, MPI_COMM_WORLD); //distribute function to processors

    MPI_Bcast(&num_samples, 1, MPI_INT, 0, MPI_COMM_WORLD); //distribute number of samples to processors

    double integral_estimate = calculate_integral(chose_function, 0.0, 1.0, num_samples, rank, size); //estimating integral

    if (rank == 0) //printing results
    {
        cout << "Estimate of the integral: " << integral_estimate << endl;
    }

    MPI_Finalize(); //Finalizing MPI

    return 0;
}