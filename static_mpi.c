#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define WORKLOAD_SIZE 8

int main(int argc, char *argv[]){
  int rank, size;
  int queue[WORKLOAD_SIZE];
  int i, tmp;

  void printArr(int *arr, int count);

  MPI_Status status;

  MPI_Init(&argc,&argv);

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // Seed rand for each process
  srand(time(NULL) + rank);

  // Generate workload of random ints in [0,4]
  if (rank == 0) {
    for (i = 0; i < WORKLOAD_SIZE; i++) {
      queue[i] = rand() % 5;
    }
    printf("[%d]\t{", rank);
    printArr(queue, WORKLOAD_SIZE);
    printf("}\n");
  }

  MPI_Finalize();
  return 0;
}

// requires count >= 1
void printArr(int *arr, int count) {
  int i;
  for (i = 0; i < count - 1; i++)
    printf("%d, ", arr[i]);
  printf("%d", arr[i]);
}
