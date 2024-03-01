#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <mpi.h>

struct Process {
    int rank;
    Process() = default;
    Process(int);
    int GetRank();
    int* operator*();
};

void Task(Process, int, long long&);

void PrimeTask(int, int);
bool IsPrime(int);

using time_env = std::chrono::milliseconds;
const int N = 1'000'000;

std::ofstream fout("output.txt");
int main() {
    Process p;
    int processes;
    long long time_elapsed;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, *p);
    MPI_Comm_size(MPI_COMM_WORLD, &processes);

    Task(p, processes, time_elapsed);
    
    if (p.GetRank() == 0) {
        std::cout << "All prime numbers under " << N << " were extracted in " << time_elapsed << " units of time using " << processes << " process(es)!" << std::endl;
    }
    MPI_Finalize();
}

Process::Process(int __r) : rank(__r) {};

int* Process::operator*() {
    return &rank;
};

int Process::GetRank() {
    return rank;
}

void Task(Process p, int processes, long long& time_elapsed) {
    int slice = N / processes;
    int start = 1;
    int end = start + slice;
    int cursor[] = {start, end};
    MPI_Status status;
    if (p.GetRank() == 0) {
        long long total_time = 0;
        auto time_start = std::chrono::steady_clock::now();

        PrimeTask(cursor[0], cursor[1]);

        auto time_end = std::chrono::steady_clock::now();
        auto time_duration = std::chrono::duration_cast<time_env>(time_end - time_start);
        auto process_time = time_duration.count();
        total_time = process_time;

        for (int i = 1; i < processes; i++) {
            cursor[0] = start = end;
            cursor[1] = end += slice;
            MPI_Send(cursor, 2, MPI_INT, i, 1, MPI_COMM_WORLD);
        }

        bool wait = true;
        while (wait) {
            int tasks_done = 0;
            for (int i = 1; i < processes; i++) {
                auto done = 0;
                MPI_Recv(&done, 1, MPI_INT, i, 3, MPI_COMM_WORLD, &status);
                if (done) {
                    tasks_done++;
                }
            }
            if (tasks_done == processes - 1) {
                wait = false;
            }
        }

        for (int i = 1; i < processes; i++) {
            long long process_time = 0;
            MPI_Recv(&process_time, 1, MPI_LONG_LONG, i, 2, MPI_COMM_WORLD, &status);
            total_time += process_time;
        }
        time_elapsed = total_time;
    }
    else {
        MPI_Recv(cursor, 2, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);

        auto time_start = std::chrono::steady_clock::now();

        PrimeTask(cursor[0], cursor[1]);

        auto time_end = std::chrono::steady_clock::now();
        auto time_duration = std::chrono::duration_cast<time_env>(time_end - time_start);

        auto process_time = time_duration.count();
        auto done = 1;
        MPI_Send(&process_time, 1, MPI_LONG_LONG, 0, 2, MPI_COMM_WORLD);
        MPI_Send(&done, 1, MPI_INT, 0, 3, MPI_COMM_WORLD);
    }
};

bool IsPrime(int n) {
    if (n == 1) {
        return false;
    }
    int div = 0;
    for (int i = 2; i * i < n; i++) {
        if (n % i == 0) {
            div++;
        }
    }
    if (div == 0) {
        return true;
    }
    return false;
}

void PrimeTask(int start, int end) {
    for (int i = start; i < end; i++) {
        if (IsPrime(i)) {
            fout << i << " ";
        }
    }
};
