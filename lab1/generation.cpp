#include <iostream>
#include <random>
#include <chrono>
#include <mpi.h>

const int M = 100;
const int N = 1000;

using time_env = std::chrono::microseconds;

int main() {
    int rank, size;
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int sum = 0;
    int m = M;

    auto start = std::chrono::steady_clock::now();

    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distribution(1, N);

        while (m--) {
            int rand = distribution(gen);
            std::cout << rand << " ";
            sum = sum + rand;
        }
    }

    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<time_env>(end - start);
    auto process_time = duration.count();

    std::cout << std::endl << "Process #" << rank << " took " << process_time
        << " microseconds to run and generated the sum of " << sum << "." << std::endl;

    MPI_Finalize();
    return 0;
}
