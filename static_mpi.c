#include <stdio.h>
#include <stdlib.h> /* First error: missing this library. This error is identified when I print the user-input arguments*/
#include <mpi.h>

int main(int argc, char *argv[]){
  int rank, size;
  double a, b;
  int n;
  double h;

  double local_a, local_b;
  int local_n;

  double partial_result = 0;
  double result;

  int i;
  double f(double x);

  MPI_Status status;

  MPI_Init(&argc,&argv);

  a = atof(argv[1]);
  b = atof(argv[2]);
  n = atoi(argv[3]);

  // printf("%lf and %lf and %d\n", a, b, n);

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  local_n = n / size;
  h = (b - a) / n;
  local_a = a + rank * local_n * h; /* After the first error is fixed, the second error appears with local_a */
  // printf("local_a %f of process %d\n", local_a, rank);
  // printf("h %f of process %d\n", h, rank);
  for (i = 0; i < local_n; i++){
    local_b = local_a + h;
    // printf("%f and %f\n", f(local_a), f(local_b));
    partial_result += (f(local_a) + f(local_b)) * h / 2;
    // printf("%f\n", partial_result);
    local_a = local_b;
  }

  MPI_Reduce(&partial_result, &result, 1, MPI_DOUBLE,MPI_SUM, 0, MPI_COMM_WORLD);
  printf("Process %d calculated %lf for partial result\n",rank, partial_result);

  if (rank == 0) printf("The final result is %f \n", result);

  MPI_Finalize();
  return 0;
}

double f(double x){
return x * x;
}
