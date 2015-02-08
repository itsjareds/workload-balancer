/* I worked on this assignment alone. I consulted a man page for MPI for how to
 create a derived MPI type based on a C struct found at the following address:

 http://mpi.deino.net/mpi_functions/MPI_Type_create_struct.html
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h> /* for memset() */
#include <mpi.h>

#define WORKLOAD_SIZE 200
#define STRUCT_LEN 3
#define NUM_TYPES 5

typedef int bool;
#define TRUE 1
#define FALSE 0

typedef struct {
  unsigned int uid;
  unsigned int i;
  double f;
} workload;

MPI_Datatype MPI_WORKLOAD;

void printArr(workload *arr, int count);
void create_MPI_Struct(MPI_Datatype *t);
unsigned int sleeptime(int i);
void compute_workload(workload *w);
int working(int queue_head);
void copy_workload(workload *src, workload *dest);
void send_workload(workload *w, int to);
int recv_workload(workload *w, int from);
int handle_workers(workload *queue, int *queue_head, bool blocking);
int workload_waiting();
void finish_worker(int worker, int *finished);

int main(int argc, char *argv[]){
  int rank, size;
  workload queue[WORKLOAD_SIZE];
  workload local_workload;
  workload *times;
  int i, j, queue_head = 0, finished = 0;
  double start, stop, total; /* timing variables */
  MPI_Status status;

  MPI_Init(&argc,&argv);

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  create_MPI_Struct(&MPI_WORKLOAD);

  start = MPI_Wtime();

  // Seed rand for each process
  srand(time(NULL) + rank);

  // Generate workload of random ints in [0,4]
  if (rank == 0) {
    times = malloc(sizeof(workload) * size);
    memset(times, 0, sizeof(workload) * size);

    for (i = 0; i < WORKLOAD_SIZE; i++) {
      queue[i].uid = i;
      queue[i].i = rand() % NUM_TYPES;
    }
    printf("[%d] MAIN WORKLOAD:\t{", rank);
    printArr(queue, WORKLOAD_SIZE);
    printf("}\n");
    fflush(stdout);

    copy_workload(&queue[queue_head++], &local_workload);
    // Deal initial round robin workloads
    for (i = 1; i < size; i++) {
      if (working(queue_head)) {
        printf("Sending %d to %d\n", queue[i].i, i);
        send_workload(&queue[i], i);
        queue_head++;
      } else {
        workload w;
        w.uid = -1;
        send_workload(&w, i);
        finish_worker(i, &finished);
      }
    }
  } else {
    recv_workload(&local_workload, 0);
    if (local_workload.uid == -1)
      queue_head = -1;
  }

  while (working(queue_head)) {
    if (rank != 0 || working(queue_head)) {
      printf("[%d] new workload:\t", rank);
      printArr(&local_workload, 1);
      printf("\n");
      fflush(stdout);

      /* Begin computing workload */
      compute_workload(&local_workload);

      printf("[%d] sleep took %.3lf sec\n", rank, local_workload.f);
      fflush(stdout);
    }

    /* Handle workload returns */
    if (rank == 0) {
      copy_workload(&local_workload, &queue[local_workload.uid]);
      if (working(queue_head))
        copy_workload(&queue[queue_head++], &local_workload);

      finished += handle_workers(queue, &queue_head, FALSE);
    } else {
      printf("[%d] returning workload %d...\n", rank, local_workload.uid);
      send_workload(&local_workload, 0);
      if (recv_workload(&local_workload, 0) == -1)
        queue_head = -1;
    }
    fflush(stdout);
  }

  if (rank == 0) {
    printf("[0] *Calling finish_worker(0)\n");
    finish_worker(0, &finished);
  }

  printf("[%d] Work done...\n", rank);

  /* Continue listening until all finished */
  while (rank == 0 && finished < size) {
    printf("[%d] Waiting for workload (finished=%d)\n", 0, finished);
    finished += handle_workers(queue, &queue_head, TRUE);
  }

  if (rank == 0)
    printf("[%d] About to exit, finished = %d\n", rank, finished);

  // /* Send stats back to master */
  // if (rank != 0) {
  //   MPI_Send(local_result, NUM_TYPES, MPI_WORKLOAD, 0, rank, MPI_COMM_WORLD);
  // } else {
  //   /* Sum master work times */
  //   for (i = 0; i < NUM_TYPES; i++) {
  //     times[0].i += local_result[i].i;
  //     times[0].f += local_result[i].f;
  //   }
  //
  //   workload recvbuf[NUM_TYPES];
  //   for (i = 1; i < size; i++) {
  //     MPI_Recv(recvbuf, NUM_TYPES, MPI_WORKLOAD, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
  //
  //     for (j = 0; j < NUM_TYPES; j++) {
  //       local_result[j].i += recvbuf[j].i;
  //       local_result[j].f += recvbuf[j].f;
  //       times[status.MPI_SOURCE].i += recvbuf[j].i;
  //       times[status.MPI_SOURCE].f += recvbuf[j].f;
  //     }
  //   }
  //
  //   printf("\n");
  //
  //   for (i = 0; i < NUM_TYPES; i++) {
  //     workload *w = &local_result[i];
  //     double avg = (w->i > 0) ? w->f / w->i : 0.0f;
  //     printf("Type %d:\tn=%d\ttot=%.3lf\tavg=%.3lf\n", i, w->i, w->f, avg);
  //   }
  //
  //   printf("\n");
  //
  //   for (i = 0; i < size; i++) {
  //     workload *w = &times[i];
  //     double avg = (w->i > 0) ? w->f / w->i : 0.0f;
  //     printf("Node %d:\tn=%d\ttot=%.3lf\tavg=%.3lf\n", i, w->i, w->f, avg);
  //   }
  //
  //   stop = MPI_Wtime();
  //   printf("\nTotal execution time: %.3lf sec\n", stop - start);
  //   free(times);
  // }

  printf("[%d] Done.\n", rank);
  MPI_Finalize();

  return 0;
}

int working(int queue_head) {
  return (queue_head >= 0 && queue_head < WORKLOAD_SIZE) ? TRUE : FALSE;
}

void copy_workload(workload *src, workload *dest) {
  dest->uid = src->uid;
  dest->i = src->i;
  dest->f = src->f;
}

void send_workload(workload *w, int to) {
  MPI_Send(w, 1, MPI_WORKLOAD, to, 0, MPI_COMM_WORLD);
}

int recv_workload(workload *w, int from) {
  MPI_Status stat;
  MPI_Recv(w, 1, MPI_WORKLOAD, from, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);

  int id = -1;
  if (w->uid >= 0 && w->uid <= WORKLOAD_SIZE)
    id = stat.MPI_SOURCE;
  return id;
}

void compute_workload(workload *w) {
  double begin, end;

  begin = MPI_Wtime();
  usleep(sleeptime(w->i));
  end = MPI_Wtime();

  w->f = end - begin;
}

int workload_waiting() {
  int flag;
  MPI_Status stat;

  MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &stat);

  return flag;
}

int handle_workers(workload *queue, int *queue_head, bool blocking) {
  int id, finished = 0;
  workload w;
  /* Check for waiting messages in the queue */
  while (workload_waiting() || blocking) {
    id = recv_workload(&w, MPI_ANY_SOURCE);
    if (id != -1) {
      copy_workload(&w, &queue[w.uid]);
      if (working(*queue_head))
        send_workload(&queue[(*queue_head)++], id);
      else {
        printf("[0] Calling finish_worker(%d)\n", id);
        finish_worker(id, &finished);
        w.uid = -1;
        send_workload(&w, id);
      }
    }

    if (blocking)
      break;
  }
  return finished;
}

void finish_worker(int id, int *finished) {
  printf("##### Finishing worker %d\n", id);
  *finished += 1;
}

/* Sleep time in us */
unsigned int sleeptime(int i) {
  unsigned int ret = 0;
  switch (i) {
    case 0:
      ret = 100000 + rand() % 2900000;
      break;
    case 1:
      ret = 2000000 + rand() % 3000000;
      break;
    case 2:
      ret = 1000000 + rand() % 5000000;
      break;
    case 3:
      ret = 5000000 + rand() % 2500000;
      break;
    case 4:
      ret = 7000000 + rand() % 2000000;
      break;
  }
  return ret;
}

/* Requires count >= 1 */
void printArr(workload *arr, int count) {
  int i;
  for (i = 0; i < count - 1; i++)
    printf("%d, ", arr[i].i);
  printf("%d", arr[i].i);
}

void create_MPI_Struct(MPI_Datatype *t) {
  MPI_Datatype types[STRUCT_LEN] = {MPI_UNSIGNED, MPI_UNSIGNED, MPI_DOUBLE};
  int blocklen[STRUCT_LEN] = {1, 1, 1};
  MPI_Aint disp[STRUCT_LEN];

  workload w;
  disp[0] = (void *)&w.uid - (void *)&w;
  disp[1] = (void *)&w.i - (void *)&w;
  disp[2] = (void *)&w.f - (void *)&w;

  MPI_Type_create_struct(STRUCT_LEN, blocklen, disp, types, t);
  MPI_Type_commit(t);
}
