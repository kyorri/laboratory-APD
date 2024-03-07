#include <iostream>
#include <mpi.h>

const int N = 10;
int v[N] = { 27, -14, 56, 0, -9, 42, 18, -3, 7, -30 };
int VAL = 7;

int slice;

bool SearchSlice(int, int, int&);
bool SequentialTask(int&);
bool ParallelTask(const int&, const int&, int&);

int main() {
    int rank, size;
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size == 1) {
        int pos;
        bool found = SequentialTask(pos);
        if (found) {
            std::cout << "Found at position " << pos << "!" << std::endl;
        }
        else {
            std::cout << "Not found." << std::endl;
        }
    }
    else {
        int pos;
        bool found = ParallelTask(rank, size, pos);
        if (found) {
            std::cout << "Found at position " << pos << "!" << std::endl;
        }
        else {
            std::cout << "Not found." << std::endl;
        }
    }

    MPI_Finalize();
    return 0;
}

bool SearchSlice(int start, int end, int& pos) {
    pos = -1;
    for (int i = start; i < end; i++) {
        if (v[i] == VAL) {
            pos = i;
            return true;
        }
    }
    return false;
};

bool SequentialTask(int &pos) {
    bool found = SearchSlice(0, N, pos);
    return found;
}

bool ParallelTask(const int& rank, const int& size, int& found_pos) {
    if (rank == 0) {
        int slice = N / size;
        int start = 0;
        int end = start + slice + N % size;

        int pos;
        bool found = SearchSlice(start, end, pos);

        if (found == true) {
            found_pos = pos;
            return true;
        }

        for (int i = 1; i < size; i++) {
            start = end;
            end = end + slice;
            int positions[2] = { start, end };
            MPI_Send(positions, 2, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        for (int i = 1; i < size; i++) {
            int pos;
            bool found;
            MPI_Status status;
            MPI_Recv(&found, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &status);
            if (found == true) {
                MPI_Recv(&pos, 1, MPI_INT, i, 2, MPI_COMM_WORLD, &status);
                found_pos = pos;
                return true;
            }
        }
        return false;
    }
    else {
        int positions[2];
        MPI_Status status;
        MPI_Recv(positions, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        int pos;
        bool found = SearchSlice(positions[0], positions[1], pos);
        MPI_Send(&found, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
        if (found == true) {
            MPI_Send(&pos, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
        }
    }
}
