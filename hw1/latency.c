#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    int rank, size;
    double start_time, end_time, latency;
    int message = 42;  // 用于测试的消息

    // 初始化MPI环境
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 2) {
        if (rank == 0) {
            printf("This program requires exactly 2 MPI processes.\n");
        }
        MPI_Finalize();
        return 1;
    }

    if (rank == 0) {
        // 进程0发送消息给进程1
        start_time = MPI_Wtime();
        MPI_Send(&message, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Recv(&message, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        end_time = MPI_Wtime();

        // 计算延迟（以微秒为单位）
        latency = (end_time - start_time) * 1000000;
        printf("Round-trip latency: %.2f usec\n", latency);
    } else if (rank == 1) {
        // 进程1接收消息并立即返回
        MPI_Recv(&message, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Send(&message, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    // 结束MPI环境
    MPI_Finalize();
    return 0;
}
